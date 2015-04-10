/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    jsclientstubgenerator.cpp
 * @date    10/22/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "jsclientstubgenerator.h"
#include <algorithm>

using namespace jsonrpc;
using namespace std;

#define TEMPLATE_JS_PROLOG "function <class>(url) {\n\
    this.url = url;\n\
    var id = 1;\n\
    \n\
    function doJsonRpcRequest(method, params, methodCall, callback_success, callback_error) {\n\
        var request = {};\n\
        if (methodCall)\n\
            request.id = id++;\n\
        request.jsonrpc = \"2.0\";\n\
        request.method = method;\n\
        request.params = params\n\
        JSON.stringify(request);\n\
        \n\
        $.ajax({\n\
            type: \"POST\",\n\
            url: url,\n\
            data: JSON.stringify(request),\n\
            success: function (response) {\n\
                if (methodCall) {\n\
                    if (response.hasOwnProperty(\"result\") && response.hasOwnProperty(\"id\")) {\n\
                        callback_success(response.id, response.result);\n\
                    } else if (response.hasOwnProperty(\"error\")) {\n\
                        if (callback_error != null)\n\
                            callback_error(response.error.code,response.error.message);\n\
                    } else {\n\
                        if (callback_error != null)\n\
                            callback_error(-32001, \"Invalid Server response: \" + response);\n\
                    }\n\
                }\n\
            },\n\
            error: function () {\n\
                if (methodCall)\n\
                    callback_error(-32002, \"AJAX Error\");\n\
            },\n\
            dataType: \"json\"\n\
        });\n\
        return id-1;\n\
    }\n\
    this.doRPC = function(method, params, methodCall, callback_success, callback_error) {\n\
        return doJsonRpcRequest(method, params, methodCall, callback_success, callback_error);\n\
    }\n\
}\n"

#define TEMPLATE_JS_METHOD "<class>.prototype.<procedure> = function(<params>callbackSuccess, callbackError) {"
#define TEMPLATE_JS_PARAM_NAMED "var params = {<params>};"
#define TEMPLATE_JS_PARAM_POSITIONAL "var params = [<params>];"
#define TEMPLATE_JS_PARAM_EMPTY "var params = null;"
#define TEMPLATE_JS_CALL_METHOD "return this.doRPC(\"<procedure>\", params, true, callbackSuccess, callbackError);"
#define TEMPLATE_JS_CALL_NOTIFICATION "this.doRPC(\"<procedure>\", params, false, callbackSuccess, callbackError);"



JSClientStubGenerator::JSClientStubGenerator(const std::string &stubname, std::vector<Procedure> &procedures, CodeGenerator &cg) : StubGenerator(stubname, procedures, cg)
{
}

string JSClientStubGenerator::class2Filename(const string &classname)
{
    string result = classname;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result + ".js";
}

void JSClientStubGenerator::generateStub()
{
    cg.write(replaceAll(TEMPLATE_JS_PROLOG, "<class>", stubname));
    cg.writeNewLine();

    for (unsigned int i=0; i < procedures.size(); i++)
    {
        this->generateMethod(procedures[i]);
    }

}

void JSClientStubGenerator::generateMethod(Procedure &proc)
{
    string method = TEMPLATE_JS_METHOD;
    replaceAll2(method, "<class>", stubname);
    replaceAll2(method, "<procedure>", proc.GetProcedureName());

    stringstream param_string;
    stringstream params_assignment;

    parameterNameList_t list = proc.GetParameters();
    for (parameterNameList_t::iterator it = list.begin(); it != list.end();)
    {
        param_string << it->first;

        if (proc.GetParameterDeclarationType() == PARAMS_BY_NAME)
            params_assignment << it->first << " : " << it->first;
        else
            params_assignment << it->first;

        if (++it != list.end())
        {
                params_assignment << ", ";
        }
        param_string << ", ";
    }

    replaceAll2(method, "<params>", param_string.str());

    cg.writeLine(method);
    cg.increaseIndentation();

    string params;

    if (proc.GetParameters().size() > 0)
    {
        if (proc.GetParameterDeclarationType() == PARAMS_BY_NAME)
            params = TEMPLATE_JS_PARAM_NAMED;
        else
            params = TEMPLATE_JS_PARAM_POSITIONAL;

        replaceAll2(params, "<params>", params_assignment.str());
        cg.writeLine(params);
    }
    else
    {
        cg.writeLine(TEMPLATE_JS_PARAM_EMPTY);
    }


    if (proc.GetProcedureType() == RPC_METHOD)
        method = TEMPLATE_JS_CALL_METHOD;
    else
        method = TEMPLATE_JS_CALL_NOTIFICATION;

    replaceAll2(method, "<procedure>", proc.GetProcedureName());

    cg.writeLine(method);

    cg.decreaseIndentation();
    cg.writeLine("};");


}
