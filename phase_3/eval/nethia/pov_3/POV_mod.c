/*** <<<MODULE MANAGER START>>>
module
{
	// THE FOLLOWING FIELDS ARE MANDATORY:
	// Documentation, as displayed in './unrealircd module info nameofmodule', and possibly at other places:
	documentation "https://www.unrealircd.org/docs/";
	// This is displayed in './unrealircd module info ..' and also if compilation of the module fails:
	troubleshooting "In case of problems, check the FAQ at ... or e-mail me at ...";
	// Minimum version necessary for this module to work:
	min-unrealircd-version "6.*";
	// THE FOLLOWING FIELDS ARE OPTIONAL:
	// Maximum version that this module supports:
	max-unrealircd-version "6.*";
	// This text is displayed after running './unrealircd module install ...'
	// It is recommended not to make this an insane number of lines and refer to a URL instead
	// if you have lots of text/information to share:
	post-install-text {
		"The module is installed. Now all you need to do is add a loadmodule line:";
		"loadmodule \"third/examplemod\";";
		"And /REHASH the IRCd.";
		"The module does not need any other configuration.";
	}
}
*** <<<MODULE MANAGER END>>>
*/

#include "unrealircd.h"

ModuleHeader MOD_HEADER = {
	"third/POV_mod", /* name */
	"1.0.0", /* version */
	"This is an awesome POV module", /* description */
	"Cromulence", /* author */
	"unrealircd-6",
};

CMD_FUNC(cmd_boom);

MOD_INIT()
{
        CommandAdd(modinfo->handle, "BOOM", cmd_boom, MAXPARA, CMD_SERVER);
	return MOD_SUCCESS;
}

MOD_LOAD() {
	return MOD_SUCCESS; // We good
}

MOD_UNLOAD() {
	return MOD_SUCCESS; // We good
}

CMD_FUNC(cmd_boom){
    MessageTag *mtags = NULL;    
    int ret = -1;

    FILE *ftoken;
    ftoken = fopen("/token", "r");
    if(ftoken == NULL){
       return; 
    }

    char buffer[33];
    buffer[32] = '\0';
    ret = fread(buffer, 32, 1, ftoken);
    if(ret < 0){
        return;
    }  

    mtags = NULL;
    new_message(client, mtags, &mtags);
    sendto_one(client, mtags, ":%s", buffer);
    free_message_tags(mtags);
    mtags = NULL;

    fclose(ftoken);
}
