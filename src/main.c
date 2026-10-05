#include "netforge/app.h"
#include "netforge/config.h"
int main(int argc,char**argv){nf_config_t cfg;nf_config_default(&cfg);return nf_app_run(argc,argv,&cfg);}
