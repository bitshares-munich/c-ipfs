#ifndef __COMMANDS_REQUEST_H__
#define __COMMANDS_REQUEST_H__

#include "context.h"
#include "command.h"

struct Request {
	char* path;
	//optmap options;
	char* arguments;
	//file[] files;
	struct Command cmd;
	struct Context* invoc_context;
	//context rctx;
	//map[string]Option optionDefs;
	//map[string]interface{} values;
	//ioReader stdin;
};

#endif /* request_h */
