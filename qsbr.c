#include <chrono>  // for time
#include <mutex>   // for std::mutex
#include <shared_mutex> // for reader-writer lock
#include <urcu-qsbr.h>  // for urcu-qsbr
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <urcu/rculist.h>
#include <urcu/compiler.h>
#include <stdio.h>

using namespace std;

void *reader_fn(void *arg);
void *writer_fn(void *arg);

pthread_t *readers,*writers;


struct mynode {
	int value;
	int ab[100];
	struct cds_list_head node;
	struct rcu_head rcu_head;
};

struct thread_input{
	int id;
	struct mynode *node;
};

CDS_LIST_HEAD(mylist);
long int nreaders,nwriters,size;
bool flag_rcu;
mutex writer_mtx;
shared_timed_mutex shrd_mtx;

static
void free_node_rcu(struct rcu_head *head)
{
	struct mynode *node = caa_container_of(head, struct mynode, rcu_head);

	free(node);
}

int main(){
	readers = (pthread_t *)malloc(sizeof(pthread_t)*100);
	writers = (pthread_t *)malloc(sizeof(pthread_t)*100);
	long int *readers_id,*writers_id;
	scanf("%ld %ld %ld",&nreaders,&nwriters,&size);
	flag_rcu = 0;	
	readers_id = (long int *)malloc(sizeof(long int)*100);
	writers_id = (long int *)malloc(sizeof(long int)*100);
	struct mynode *node,*n;

	for(int i = 0;i < size;i++){
		node = (struct mynode *)malloc(sizeof(*node));
       		if(!node){

		}
		node->value = i;
		for(int i = 0;i < 100;i++){
			node->ab[i] = i;
		}
		cds_list_add_rcu(&node->node, &mylist);
	}

	auto start = std::chrono::high_resolution_clock::now();
	long int cr=0,cw=0,flg = 0 ; // flg is set means writer else reader
	for(long i = 0;i < 10;i++){		
		readers_id[i] = i;
		pthread_create(&readers[i],NULL,reader_fn,&readers_id[i]);

		cr++;
	}
	for(int j = 10;j < 20;j++){
		writers_id[j] = j;
		pthread_create(&writers[j],NULL,writer_fn,&writers_id[j]);	
		cw++;	
	}
	
	for(long i = 0;i < 10;i++){
		pthread_join(readers[i],NULL);
	}
	for(long i = 10;i < 20;i++){
		pthread_join(writers[i],NULL);
	}
	auto finish = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> elapsed = finish - start;
	std::cout << "Elapsed time: " << elapsed.count() << " s\n";
//sleep(1000);	
	return 0;
}

void *reader_fn(void *arg){
	
	long int t_id = *(long *)arg;
	struct mynode *node;
	rcu_register_thread();
	if(t_id == 1){
		while (1);
	}
	for(int i = 0;i < nreaders;i++){
		cds_list_for_each_entry(node, &mylist, node) {
			int temp =  node->value;
		}
		
	}
		rcu_quiescent_state();
}

void *writer_fn(void *arg){
	int t_id = *(int *)arg;
	struct mynode *node,*n;
	rcu_register_thread();
	for(int i = 0; i < nwriters;i++){
		usleep(100);
		unique_lock<shared_timed_mutex> lock(shrd_mtx);
		
			cds_list_for_each_entry_safe(node, n, &mylist, node) {
				if(node->value/10 == t_id/10){
					struct mynode *new_node;
					new_node = (struct mynode *)malloc(sizeof(*node));
					if (!new_node) {
					}
					/* Replacement node value is negated original value. */
					new_node->value = node->value;
					for(int i = 0;i < 100;i++){
						new_node->ab[i] = i;
					}
					cds_list_replace_rcu(&node->node, &new_node->node);
					call_rcu(&node->rcu_head, free_node_rcu);
				}
			}	
		rcu_quiescent_state();
	}
}
