#include "include/resource_negotiation.h"
#include "include/noc_functions.h"
#include "include/apps.h"

extern app my_app;
int offer_cores_original(core_list *cores, app req_app, region req_reg, int *Offered_cores, int req_id);
int offer_cores_updated(core_list *cores, app req_app, region req_reg, int *Offered_cores, int req_id);
int offer_cores_fft_original(core_list *cores, app req_app, region req_reg, int *Offered_cores, int req_id);

int offer_cores(core_list *cores, app req_app, region req_reg, int *Offered_cores, int req_id) {

#ifdef RESOURCE_ALGO_ORIG
	if (executed_app != FFT) {
		return offer_cores_original(cores, req_app, req_reg, Offered_cores, req_id);
	} else {
		return offer_cores_fft_original(cores, req_app, req_reg, Offered_cores, req_id);
	}
#elif RESOURCE_ALGO_UPDATED
	return offer_cores_updated(cores, req_app, req_reg, Offered_cores, req_id);
#elif RESOURCE_ALGO_UPDATED_GENEROUS
	return offer_cores_updated(cores, req_app, req_reg, Offered_cores, req_id);
#else
	return offer_cores_original(cores, req_app, req_reg, Offered_cores, req_id);
#endif
}

/* Speedup is calculated correclty inside Speedup function */
int offer_cores_original(core_list *cores, app req_app, region req_reg, int *Offered_cores, int req_id) {
	int Of_cores_num=0, min_dist=0, cur_dist=0;
	float gain_total=0.1, base_receiver=0.0, base_giver=0.0, gain_receiver=0.0, loss_giver=0.0, share_giver=0.0, new_gain=0.0;
	int Cores_receiver = req_app.num_of_cores, Cores_giver = my_app.num_of_cores;
	core_list *tmp, *GreedyChoice;
	int offered_cnt=0, counted_cores=0;

	for (tmp=cores; tmp!=NULL; tmp=tmp->next) {
					if (distance(req_reg.C, tmp->core_id) <= req_reg.r) share_giver++;
					counted_cores++;
					if (tmp->offered_to != -1) offered_cnt++;
					fprintf(log_file,"Core %d is offered to %d\n",tmp->core_id,tmp->offered_to);
	}

	if (offered_cnt == (counted_cores-2) && my_idag != -1) {
					fprintf(log_file,"I did not give up my only not offered core\n");
					return 0;
	}

	share_giver = share_giver / (float) region_count(req_reg);

	if (my_idag == -1) {
		while (gain_total > 0.0) {
			gain_total = 0.0;
			GreedyChoice = NULL;//-1;
			min_dist = -1;
			base_giver = 0;
			tmp = cores->next;//very important!!! that way i avoid giving up my agent core

			while (tmp != NULL) {
				cur_dist = distance(req_reg.C, tmp->core_id);
				if (tmp->offered_to == -1 && cur_dist <= req_reg.r) {
					//Of_cores_num == 0 to be the first offered core
					//Cores_receiver == 0 to avoid providing the core to an non-initial core search
					if (low_voltage_core[tmp->core_id] && Of_cores_num == 0 && Cores_receiver == 0) {
						if ((Cores_receiver + Of_cores_num) == 0) {
										gain_receiver = 1000; //0 sto init_app
						} else if ((Cores_receiver + Of_cores_num) == 1) {
										gain_receiver = 100; //no worker cores  
						} else { /* (Cores_receiver + Of_cores_num) > 1 */
										base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
										//gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver); 
										gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver);
						}

						loss_giver = 0;
						new_gain = gain_receiver - loss_giver;
						gain_total = new_gain;
						GreedyChoice = tmp;//->core_id;
						break;
		#ifdef LOW_VOLTAGE_ISLANDS_4
		} else if (low_voltage_core[tmp->core_id] && Of_cores_num == 0 && Cores_receiver == 1) {
			if (Cores_receiver == 0 && Of_cores_num == 0) gain_receiver = 1000; //0 sto init_app                            
			else gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver);

			loss_giver = 0;

			new_gain = gain_receiver - loss_giver;
			gain_total = new_gain;
			GreedyChoice = tmp;//->core_id;
			break;
		#endif
		} else if (low_voltage_core[tmp->core_id] == 0) {
						if ((Cores_receiver + Of_cores_num) == 0) {
										gain_receiver = 1000; //0 sto init_app
						} else if ((Cores_receiver + Of_cores_num) == 1) {
										gain_receiver = 100; //no worker cores  
						} else { /* (Cores_receiver + Of_cores_num) > 1 */
										base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
										gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver);
						}

						loss_giver = 0;
						new_gain = gain_receiver - loss_giver;
						if (new_gain > gain_total){
										gain_total = new_gain;
										min_dist = cur_dist;
										GreedyChoice = tmp;//->core_id;
						} else if (new_gain == gain_total && cur_dist < min_dist) {
										//printf("I am %d and i change offer to %d with cores %d->%d with distances %d->%d\n",
										//      node_id,req_id,GreedyChoice->core_id,tmp->core_id,min_dist,cur_dist);
										min_dist = cur_dist;
										GreedyChoice = tmp;
						}
					}
				}

				tmp = tmp->next;
			}

			if (gain_total > 0.0) {
				Offered_cores[Of_cores_num++] = GreedyChoice->core_id;
				GreedyChoice->offered_to = req_id;
			}
		}
	}
#ifndef GREEDY_MANAGER
	else {
		while (gain_total > 0.0) {
			gain_total = 0.0;
			GreedyChoice = NULL;//-1;
			min_dist = -1;
			base_giver = Speedup(my_app, Cores_giver - Of_cores_num);

			tmp = cores->next->next;//very important!!! that way i avoid giving up my only working core

			while (tmp != NULL) {
				if (core_inter_head[tmp->core_id] != NULL &&
					(core_inter_head[tmp->core_id]->type == INIT_WORK_NODE_PENDING || core_inter_head[tmp->core_id]->type == INIT_WORK_NODE)) {
						fprintf(log_file,"Core %d is about to start work type = %d\n",tmp->core_id,core_inter_head[tmp->core_id]->type);
						tmp = tmp->next;
				} else {
					cur_dist = distance(req_reg.C, tmp->core_id);
					if (tmp->offered_to == -1 && cur_dist <= req_reg.r) {
						if ((Cores_receiver + Of_cores_num) == 0) {
										gain_receiver = 1000; //0 sto init_app
						} else if ((Cores_receiver + Of_cores_num) == 1) {
										gain_receiver = 100; //no worker cores -- in case I have only one worker core then I should not be here 
						} else { /* (Cores_receiver + Of_cores_num) > 1 */
										base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
										gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver); /*  + 1 is ommited due to workload convention */
						}

						loss_giver = base_giver - Speedup(my_app, Cores_giver - (Of_cores_num + 1));

						new_gain = gain_receiver - loss_giver;
						if (new_gain > gain_total){
										gain_total = new_gain;
										min_dist = cur_dist;
										GreedyChoice = tmp;//->core_id;
						} else if (new_gain == gain_total && cur_dist < min_dist) {
										//printf("I am %d and i change offer to %d with cores %d->%d with distances %d->%d\n",
										//      node_id,req_id,GreedyChoice->core_id,tmp->core_id,min_dist,cur_dist);
										min_dist = cur_dist;
										GreedyChoice = tmp;
						}
					}

					tmp = tmp->next;
				}
			}

			if (gain_total > 0.0) {
							Offered_cores[Of_cores_num++] = GreedyChoice->core_id;
							GreedyChoice->offered_to = req_id;
			}
		}
	}
	#endif
	fprintf(log_file,"I will offer %d cores\n",Of_cores_num);
        
	return Of_cores_num;
}
       

/* Speedup is calculated correclty inside Speedup function */
int offer_cores_updated(core_list *cores, app req_app, region req_reg, int *Offered_cores, int req_id) {
	int Of_cores_num=0, min_dist=0, cur_dist=0, offered_cnt=0, counted_cores=0;
	float gain_total=0.1, base_receiver=0.0, base_giver=0.0, gain_receiver=0.0, loss_giver=0.0, new_gain=0.0;
	int Cores_receiver = req_app.num_of_cores, Cores_giver = my_app.num_of_cores;
	core_list *tmp, *GreedyChoice;

	if (my_idag != -1) {
		for (tmp=cores; tmp!=NULL; tmp=tmp->next) {
			counted_cores++;
			if (tmp->offered_to != -1) offered_cnt++;
			fprintf(log_file,"Core %d is offered to %d\n",tmp->core_id,tmp->offered_to);	
		}

		if (offered_cnt == (counted_cores-2)) {
			fprintf(log_file,"I did not give up my only not offered core\n");
			return 0;
		}
	}

	if (my_idag == -1) {
		while (gain_total > 0.0) {
			gain_total = 0.0;
			GreedyChoice = NULL;//-1;
			min_dist = -1;
			base_giver = 0; 		
			tmp = cores->next;//very important!!! that way i avoid giving up my agent core
			
			while (tmp != NULL) {
				cur_dist = distance(req_reg.C, tmp->core_id);
				if (tmp->offered_to == -1 && cur_dist <= req_reg.r) {
					//Of_cores_num == 0 to be the first offered core
					//Cores_receiver == 0 to avoid providing the core to an non-initial core search
					if (low_voltage_core[tmp->core_id] && Of_cores_num == 0 && Cores_receiver == 0) {
						if ((Cores_receiver + Of_cores_num) == 0) {
							gain_receiver = 1000; //0 sto init_app
						} else if ((Cores_receiver + Of_cores_num) == 1) {
							gain_receiver = 100; //no worker cores	
						} else { /* (Cores_receiver + Of_cores_num) > 1 */
							base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
							gain_receiver = Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver; 
						}
						
						loss_giver = 0;
						new_gain = gain_receiver - loss_giver;
						gain_total = new_gain;
						GreedyChoice = tmp;//->core_id;
						break;
					} else if (low_voltage_core[tmp->core_id] == 0) {  
						if ((Cores_receiver + Of_cores_num) == 0) {
							gain_receiver = 1000; //0 sto init_app
						} else if ((Cores_receiver + Of_cores_num) == 1) {
							gain_receiver = 100; //no worker cores	
						} else { /* (Cores_receiver + Of_cores_num) > 1 */
							base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
							gain_receiver = Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver;
						}			
					
						loss_giver = 0;
						new_gain = gain_receiver - loss_giver;
						if (new_gain > gain_total){
							gain_total = new_gain;
							min_dist = cur_dist;
							GreedyChoice = tmp;
						} else if (new_gain == gain_total && cur_dist < min_dist) {
							min_dist = cur_dist;
							GreedyChoice = tmp;
						}
					}	
				}

				tmp = tmp->next;
			}

			if (gain_total > 0.0) {
				Offered_cores[Of_cores_num++] = GreedyChoice->core_id;
				GreedyChoice->offered_to = req_id;
			}
		}
	}
	#ifndef GREEDY_MANAGER
	else {
		while (gain_total > 0.0) {
			gain_total = 0.0;
			GreedyChoice = NULL;//-1;
			min_dist = -1;
			base_giver = Speedup(my_app, Cores_giver - Of_cores_num);
		
			tmp = cores->next->next;//very important!!! that way i avoid giving up my only working core
	
			while (tmp != NULL) {
				if (core_inter_head[tmp->core_id] != NULL && 
					(core_inter_head[tmp->core_id]->type == INIT_WORK_NODE_PENDING || core_inter_head[tmp->core_id]->type == INIT_WORK_NODE)) { 
					fprintf(log_file,"Core %d is about to start work type = %d\n",tmp->core_id,core_inter_head[tmp->core_id]->type);
					tmp = tmp->next;
				} else {
					cur_dist = distance(req_reg.C, tmp->core_id);
					if (tmp->offered_to == -1 && cur_dist <= req_reg.r) {
						if ((Cores_receiver + Of_cores_num) == 0) {
							gain_receiver = 1000; //0 sto init_app
						} else if ((Cores_receiver + Of_cores_num) == 1) {
							gain_receiver = 100; //no worker cores -- in case I have only one worker core then I should not be here	
						} else { /* (Cores_receiver + Of_cores_num) > 1 */
							base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
							gain_receiver = Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver; /*  + 1 is ommited due to workload convention */
						}
						
						loss_giver = base_giver - Speedup(my_app, Cores_giver - (Of_cores_num + 1));
					
						new_gain = gain_receiver - loss_giver;
						if (new_gain > gain_total){
							gain_total = new_gain;
							min_dist = cur_dist;
							GreedyChoice = tmp;
						} else if (new_gain == gain_total && cur_dist < min_dist) {
							min_dist = cur_dist;
							GreedyChoice = tmp;
						}
					}

					tmp = tmp->next;
				}
			}

			if (gain_total > 0.0) {
				#ifdef RESOURCE_ALGO_UPDATED_GENEROUS
				if ((Cores_receiver + Of_cores_num) > 1) {
					if (get_times(my_app, Cores_giver - (Of_cores_num + 1)) > get_times(req_app, Cores_receiver + Of_cores_num + 1)) { 
						Offered_cores[Of_cores_num++] = GreedyChoice->core_id;
						GreedyChoice->offered_to = req_id;
						fprintf(log_file,"Accepted bargain with giver_times %d receiver_times %d, Cores_giver %d, Cores_receiver %d, Of_cores_num %d\n",
							get_times(my_app, Cores_giver - (Of_cores_num + 1)), get_times(req_app, Cores_receiver + Of_cores_num + 1), Cores_giver, Cores_receiver, Of_cores_num);
					} else {
						gain_total = 0.0;
						fprintf(log_file,"Refused bargain with giver_times %d receiver_times %d, Cores_giver %d, Cores_receiver %d, Of_cores_num %d\n",
                                                	get_times(my_app, Cores_giver - (Of_cores_num + 1)), get_times(req_app, Cores_receiver + Of_cores_num + 1), Cores_giver, Cores_receiver, Of_cores_num);
					}
				} else {
					Offered_cores[Of_cores_num++] = GreedyChoice->core_id;
                                        GreedyChoice->offered_to = req_id;
				}
				#else
					if (get_times(my_app, Cores_giver - (Of_cores_num + 1)) > get_times(req_app, Cores_receiver + Of_cores_num + 1)) {
						Offered_cores[Of_cores_num++] = GreedyChoice->core_id;
						GreedyChoice->offered_to = req_id;
						fprintf(log_file,"Accepted bargain with giver_times %d receiver_times %d, Cores_giver %d, Cores_receiver %d, Of_cores_num %d\n",
										get_times(my_app, Cores_giver - (Of_cores_num + 1)), get_times(req_app, Cores_receiver + Of_cores_num + 1), Cores_giver, Cores_receiver, Of_cores_num);
					} else {
						gain_total = 0.0;
						fprintf(log_file,"Refused bargain with giver_times %d receiver_times %d, Cores_giver %d, Cores_receiver %d, Of_cores_num %d\n",
										get_times(my_app, Cores_giver - (Of_cores_num + 1)), get_times(req_app, Cores_receiver + Of_cores_num + 1), Cores_giver, Cores_receiver, Of_cores_num);
					}
				#endif
			}
		}
	}
	#endif

	fprintf(log_file,"I will offer %d cores\n",Of_cores_num);
	
	return Of_cores_num;
}

int offer_cores_fft_original(core_list *cores, app req_app, region req_reg, int *Offered_cores, int req_id) {
	int Of_cores_num=0, min_dist=0, cur_dist=0;
	float gain_total=0.1,base_receiver=0.0,base_giver=0.0,gain_receiver=0.0,loss_giver=0.0,share_giver=0.0,new_gain=0.0;
	int Cores_receiver = req_app.num_of_cores, Workers_giver = my_app.num_of_cores-1;
	core_list *tmp, *GreedyChoice;
	int offered_cnt=0, counted_cores=0;

	for (tmp=cores; tmp!=NULL; tmp=tmp->next) {
		if (distance(req_reg.C, tmp->core_id) <= req_reg.r) share_giver++;
		counted_cores++;
		if (tmp->offered_to != -1) offered_cnt++;
		fprintf(log_file,"Core %d is offered to %d\n",tmp->core_id,tmp->offered_to);	
	}

	if (offered_cnt == (counted_cores-2) && my_idag != -1) {
		fprintf(log_file,"I did not give up my only not offered core\n");
		return 0;
	}

	if (Cores_receiver == 2) {
		fprintf(log_file,"Receiver already has two cores\n");
                return 0;
	}

	share_giver = share_giver / (float) region_count(req_reg);

	if (my_idag == -1) {
		while ((gain_total > 0.0) && ((Cores_receiver + Of_cores_num) <= 1)) {
			gain_total = 0.0;
			GreedyChoice = NULL;
			min_dist = -1;
			base_giver = 0; 		
			tmp = cores->next; //very important!!! that way i avoid giving up my agent core
			
			while (tmp != NULL) {
				cur_dist = distance(req_reg.C, tmp->core_id);
				if (tmp->offered_to == -1 && cur_dist <= req_reg.r) {
					//Of_cores_num == 0 to be the first offered core
					//Cores_receiver == 0 to avoid providing the core to an non-initial core search
					if (low_voltage_core[tmp->core_id] && Of_cores_num == 0 && Cores_receiver == 0) {
						if ((Cores_receiver + Of_cores_num) == 0) {
							gain_receiver = 1000; //0 sto init_app
						} else if ((Cores_receiver + Of_cores_num) == 1) {
							gain_receiver = 100; //no worker cores -- in case I have only one worker core then I should not be here	
						} else { /* (Cores_receiver + Of_cores_num) > 1 */
							base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
							//gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver); 
							gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver);						
						}
					
						loss_giver = 0;
						new_gain = gain_receiver - loss_giver;
						gain_total = new_gain;
						GreedyChoice = tmp;//->core_id;
						break;
					#ifdef LOW_VOLTAGE_ISLANDS_4
					} else if (low_voltage_core[tmp->core_id] && Of_cores_num == 0 && Cores_receiver == 1) {
							if (Cores_receiver == 0 && Of_cores_num == 0) gain_receiver = 1000; //0 sto init_app                            
							else gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver); /* +1 stands for the possibly offered core */

							loss_giver = 0;
							new_gain = gain_receiver - loss_giver;
							gain_total = new_gain;
							GreedyChoice = tmp;//->core_id;
							break;
					#endif
					} else if (low_voltage_core[tmp->core_id] == 0) {  
						if ((Cores_receiver + Of_cores_num) == 0) {
							gain_receiver = 1000; //0 sto init_app
						} else if ((Cores_receiver + Of_cores_num) == 1) {
							gain_receiver = 100; //no worker cores -- in case I have only one worker core then I should not be here	
						} else { /* (Cores_receiver + Of_cores_num) > 1 */
							base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
                            				gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver);
						}

						loss_giver = 0;
						new_gain = gain_receiver - loss_giver;
						if (new_gain > gain_total){
							gain_total = new_gain;
							min_dist = cur_dist;
							GreedyChoice = tmp;//->core_id;
						} else if (new_gain == gain_total && cur_dist < min_dist) {
							min_dist = cur_dist;
							GreedyChoice = tmp;
						}
					}	
				}

				tmp = tmp->next;
			}

			if (gain_total > 0.0) {
				Offered_cores[Of_cores_num++] = GreedyChoice->core_id;
				GreedyChoice->offered_to = req_id;
			}
		}
		
		/* FFT app requires only power of 2 exec cores plus its manager 
		 * I do not include higher than 5 because it will create no speedup
		 */
		if ((Cores_receiver + Of_cores_num) == 4) {
			for (tmp = my_cores->next; tmp!=NULL; tmp=tmp->next) {
				if (tmp->core_id == Offered_cores[Of_cores_num-1]) {
						fprintf(log_file,"Abandoning offered core %d because FFT needs 2 cores\n",tmp->core_id);
						tmp->offered_to = -1;
						Of_cores_num--;
						break;
				}	
			}	
		} 
		/*
		else if (Of_cores_num > 4) {
			
		}
		*/
	}
	#ifndef GREEDY_MANAGER
	else {
		
		if (((Workers_giver == 4) && (Cores_receiver < 3)) || ((Workers_giver == 2) && (Cores_receiver < 2))) {
		
			while (gain_total > 0.0) {
				gain_total = 0.0;
				GreedyChoice = NULL;//-1;
				min_dist = -1;
				base_giver = Speedup(my_app, Workers_giver - Of_cores_num);
			
				tmp = cores->next->next;//very important!!! that way i avoid giving up my only working core
		
				while (tmp != NULL) {
					if (core_inter_head[tmp->core_id] != NULL && 
						(core_inter_head[tmp->core_id]->type == INIT_WORK_NODE_PENDING || core_inter_head[tmp->core_id]->type == INIT_WORK_NODE)) { 
						fprintf(log_file,"Core %d is about to start work type = %d\n",tmp->core_id,core_inter_head[tmp->core_id]->type);
						tmp = tmp->next;
					} else {
						cur_dist = distance(req_reg.C, tmp->core_id);
						if (tmp->offered_to == -1 && cur_dist <= req_reg.r) {
							if ((Cores_receiver + Of_cores_num) == 0) {
								gain_receiver = 1000; //0 sto init_app
							} else if ((Cores_receiver + Of_cores_num) == 1) {
								gain_receiver = 100; //no worker cores -- in case I have only one worker core then I should not be here	
							} else { /* (Cores_receiver + Of_cores_num) > 1 */
								base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
                                                        	gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver); /*  + 1 is ommited due to workload convention */
							}			
						
							loss_giver = base_giver - Speedup(my_app, Workers_giver - (Of_cores_num + 1));
						
							new_gain = gain_receiver - loss_giver;
							if (new_gain > gain_total){
								gain_total = new_gain;
								min_dist = cur_dist;
								GreedyChoice = tmp;//->core_id;
							} else if (new_gain == gain_total && cur_dist < min_dist) {
								//printf("I am %d and i change offer to %d with cores %d->%d with distances %d->%d\n",
								//	node_id,req_id,GreedyChoice->core_id,tmp->core_id,min_dist,cur_dist);
								min_dist = cur_dist;
								GreedyChoice = tmp;
							}
						}

						tmp = tmp->next;
					}
				}

				if (gain_total > 0.0) {
					Offered_cores[Of_cores_num++] = GreedyChoice->core_id;
					GreedyChoice->offered_to = req_id;
				}
			}
		
			if ((Cores_receiver + Of_cores_num) == 4) {
				for (tmp = my_cores->next; tmp!=NULL; tmp=tmp->next) {
					if (tmp->core_id == Offered_cores[Of_cores_num-1]) {
							fprintf(log_file,"Abandoning offered core %d because FFT needs 2 cores\n",tmp->core_id);
							tmp->offered_to = -1;
							Of_cores_num--;
							break;
					}	
				}	
			}
			
		}	
	}	
	#endif

	/* FFT app requires only power of 2 exec cores plus its manager */
	/*
	if (my_idag == -1) {
	
	if (executed_app == FFT) {
		if (Of_cores_num == 3) {
			for (tmp = my_cores->next; tmp!=NULL; tmp=tmp->next) {
				if (tmp->core_id == Offered_cores[Of_cores_num-1]) {
						fprintf(log_file,"Abandoning offered core %d because FFT needs 2 cores\n",tmp->core_id);
						tmp->offered_to = -1;
						Of_cores_num--;
						break;
				}	
			}	
		} else if (Of_cores_num > 4) {
			
		}	
	}	
	*/
	return Of_cores_num;
}

int offer_cores_fft_updated(core_list *cores, app req_app, region req_reg, int *Offered_cores, int req_id) { /* 26.5.2017 - I am not sure if it is correct. Has not been used to measure F_min_max0 */
	int Of_cores_num=0, min_dist=0, cur_dist=0;
	float gain_total=0.1,base_receiver=0.0,base_giver=0.0,gain_receiver=0.0,loss_giver=0.0,share_giver=0.0,new_gain=0.0;
	int Cores_receiver = req_app.num_of_cores, Workers_giver = my_app.num_of_cores-1;
	core_list *tmp, *GreedyChoice;
	int offered_cnt=0, counted_cores=0;

	for (tmp=cores; tmp!=NULL; tmp=tmp->next) {
		if (distance(req_reg.C, tmp->core_id) <= req_reg.r) share_giver++;
		counted_cores++;
		if (tmp->offered_to != -1) offered_cnt++;
		fprintf(log_file,"Core %d is offered to %d\n",tmp->core_id,tmp->offered_to);	
	}

	if (offered_cnt == (counted_cores-2) && my_idag != -1) {
		fprintf(log_file,"I did not give up my only not offered core\n");
		return 0;
	}
	share_giver = share_giver / (float) region_count(req_reg);

	if (my_idag == -1) {
		while ((gain_total > 0.0) && ((Cores_receiver + Of_cores_num) <= 2)) {
			gain_total = 0.0;
			GreedyChoice = NULL;//-1;
			min_dist = -1;
			base_giver = 0; 		
			tmp = cores->next;//very important!!! that way i avoid giving up my agent core
			
			while (tmp != NULL) {
				cur_dist = distance(req_reg.C, tmp->core_id);
				if (tmp->offered_to == -1 && cur_dist <= req_reg.r) {
					//Of_cores_num == 0 to be the first offered core
					//Cores_receiver == 0 to avoid providing the core to an non-initial core search
					if (low_voltage_core[tmp->core_id] && Of_cores_num == 0 && Cores_receiver == 0) {
						if ((Cores_receiver + Of_cores_num) == 0) {
							gain_receiver = 1000; //0 sto init_app
						} else if ((Cores_receiver + Of_cores_num) == 1) {
							gain_receiver = 100; //no worker cores -- in case I have only one worker core then I should not be here	
						} else { /* (Cores_receiver + Of_cores_num) > 1 */
							base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
                            				//gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver); 
                            				gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver);						
						}
					
						loss_giver = 0;
						new_gain = gain_receiver - loss_giver;
						gain_total = new_gain;
						GreedyChoice = tmp;//->core_id;
						break;
					#ifdef LOW_VOLTAGE_ISLANDS_4
					} else if (low_voltage_core[tmp->core_id] && Of_cores_num == 0 && Cores_receiver == 1) {
							if (Cores_receiver == 0 && Of_cores_num == 0) gain_receiver = 1000; //0 sto init_app                            
							else gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver); /* +1 stands for the possibly offered core */

							loss_giver = 0;
							new_gain = gain_receiver - loss_giver;
							gain_total = new_gain;
							GreedyChoice = tmp;//->core_id;
							break;
					#endif
					} else if (low_voltage_core[tmp->core_id] == 0) {  
						if ((Cores_receiver + Of_cores_num) == 0) {
							gain_receiver = 1000; //0 sto init_app
						} else if ((Cores_receiver + Of_cores_num) == 1) {
							gain_receiver = 100; //no worker cores -- in case I have only one worker core then I should not be here	
						} else { /* (Cores_receiver + Of_cores_num) > 1 */
							base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
                            				gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver);
						}

						loss_giver = 0;
						new_gain = gain_receiver - loss_giver;
						if (new_gain > gain_total){
							gain_total = new_gain;
							min_dist = cur_dist;
							GreedyChoice = tmp;//->core_id;
						} else if (new_gain == gain_total && cur_dist < min_dist) {
							//printf("I am %d and i change offer to %d with cores %d->%d with distances %d->%d\n",
							//	node_id,req_id,GreedyChoice->core_id,tmp->core_id,min_dist,cur_dist);
							min_dist = cur_dist;
							GreedyChoice = tmp;
						}
					}	
				}

				tmp = tmp->next;
			}

			if (gain_total > 0.0) {
				Offered_cores[Of_cores_num++] = GreedyChoice->core_id;
				GreedyChoice->offered_to = req_id;
			}
		}
		
		/* FFT app requires only power of 2 exec cores plus its manager 
		 * I do not include higher than 5 because it will create no speedup
		 */
		if ((Cores_receiver + Of_cores_num) == 4) {
			for (tmp = my_cores->next; tmp!=NULL; tmp=tmp->next) {
				if (tmp->core_id == Offered_cores[Of_cores_num-1]) {
						fprintf(log_file,"Abandoning offered core %d because FFT needs 2 cores\n",tmp->core_id);
						tmp->offered_to = -1;
						Of_cores_num--;
						break;
				}	
			}	
		} 
		/*
		else if (Of_cores_num > 4) {
			
		}
		*/
	}
	#ifndef GREEDY_MANAGER
	else {
		
		if (((Workers_giver == 4) && (Cores_receiver < 3)) || ((Workers_giver == 2) && (Cores_receiver < 2))) {
		
			while (gain_total > 0.0) {
				gain_total = 0.0;
				GreedyChoice = NULL;//-1;
				min_dist = -1;
				base_giver = Speedup(my_app, Workers_giver - Of_cores_num);
			
				tmp = cores->next->next;//very important!!! that way i avoid giving up my only working core
		
				while (tmp != NULL) {
					if (core_inter_head[tmp->core_id] != NULL && 
						(core_inter_head[tmp->core_id]->type == INIT_WORK_NODE_PENDING || core_inter_head[tmp->core_id]->type == INIT_WORK_NODE)) { 
						fprintf(log_file,"Core %d is about to start work type = %d\n",tmp->core_id,core_inter_head[tmp->core_id]->type);
						tmp = tmp->next;
					} else {
						cur_dist = distance(req_reg.C, tmp->core_id);
						if (tmp->offered_to == -1 && cur_dist <= req_reg.r) {
							if ((Cores_receiver + Of_cores_num) == 0) {
								gain_receiver = 1000; //0 sto init_app
							} else if ((Cores_receiver + Of_cores_num) == 1) {
								gain_receiver = 100; //no worker cores -- in case I have only one worker core then I should not be here	
							} else { /* (Cores_receiver + Of_cores_num) > 1 */
								base_receiver = Speedup(req_app, Cores_receiver + Of_cores_num);
								gain_receiver = share_giver * (Speedup(req_app, Cores_receiver + Of_cores_num + 1) - base_receiver); /*  + 1 is ommited due to workload convention */
							}			
						
							loss_giver = base_giver - Speedup(my_app, Workers_giver - (Of_cores_num + 1));
						
							new_gain = gain_receiver - loss_giver;
							if (new_gain > gain_total){
								gain_total = new_gain;
								min_dist = cur_dist;
								GreedyChoice = tmp;//->core_id;
							} else if (new_gain == gain_total && cur_dist < min_dist) {
								//printf("I am %d and i change offer to %d with cores %d->%d with distances %d->%d\n",
								//	node_id,req_id,GreedyChoice->core_id,tmp->core_id,min_dist,cur_dist);
								min_dist = cur_dist;
								GreedyChoice = tmp;
							}
						}

						tmp = tmp->next;
					}
				}

				if (gain_total > 0.0) {
					Offered_cores[Of_cores_num++] = GreedyChoice->core_id;
					GreedyChoice->offered_to = req_id;
				}
			}
		
			if ((Cores_receiver + Of_cores_num) == 4) {
				for (tmp = my_cores->next; tmp!=NULL; tmp=tmp->next) {
					if (tmp->core_id == Offered_cores[Of_cores_num-1]) {
							fprintf(log_file,"Abandoning offered core %d because FFT needs 2 cores\n",tmp->core_id);
							tmp->offered_to = -1;
							Of_cores_num--;
							break;
					}	
				}	
			}
			
		}	
	}	
	#endif

	/* FFT app requires only power of 2 exec cores plus its manager */
	/*
	if (my_idag == -1) {
	
	if (executed_app == FFT) {
		if (Of_cores_num == 3) {
			for (tmp = my_cores->next; tmp!=NULL; tmp=tmp->next) {
				if (tmp->core_id == Offered_cores[Of_cores_num-1]) {
						fprintf(log_file,"Abandoning offered core %d because FFT needs 2 cores\n",tmp->core_id);
						tmp->offered_to = -1;
						Of_cores_num--;
						break;
				}	
			}	
		} else if (Of_cores_num > 4) {
			
		}	
	}	
	*/
	return Of_cores_num;
}
