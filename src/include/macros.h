#ifndef __MACROS_H__
#define __MACROS_H__

#define FOR_MY_CORES_LIST for(tmp_core_list=my_cores;tmp_core_list!=NULL;tmp_core_list=tmp_core_list->next)
#define FOR_MY_DDS_LIST for(tmp_dds=DDS;tmp_dds!=NULL;tmp_dds=tmp_dds->next)
#define FOR_MY_COWORKERS_LIST for(tmp_cowork_list=coworkers;tmp_cowork_list!=NULL;tmp_cowork_list=tmp_cowork_list->next)

#define FOR_NUES for(k=0;k<X_max*Y_max;k++)
#endif
