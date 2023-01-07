#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define NONE -1

char* file_name;

int caller_ret_i;
char* caller_name;

typedef struct {
    char* command;
    char* arg1;
    char* arg2; 
} tokenizer;

typedef enum {
    PUSH, ADD, SUB,
    NEG, EQ, GT, LT, 
    AND, OR, NOT, POP,
    LABEL, IF_GOTO, GOTO,
    CALL, FUNCTION, RETURN
} parser_command_type;

typedef enum {
   CONSTANT, LOCAL, ARGUMENT,
   THIS, THAT, TEMP,
   STATIC, POINTER 
} parser_arg1_type;

typedef struct {
    parser_command_type command_type;
    parser_arg1_type arg1_type;
} parser;

void prepare_FILE_objects(FILE** in, FILE** out, char* argv[])
{
    char file_name_with_slash[256];
    sscanf(*(argv+1), "%[^.]", file_name_with_slash);

    char* ptr = strrchr(file_name_with_slash, '/');
    file_name = strdup(ptr + 1);

    char file_name_vm_addr[101];
    strcpy(file_name_vm_addr, file_name_with_slash);
    strcat(file_name_vm_addr, ".vm");

    char file_name_asm_addr[101];
    strcpy(file_name_asm_addr, file_name_with_slash);
    strcat(file_name_asm_addr, ".asm");

    *in = fopen(file_name_vm_addr, "r");
    *out = fopen(file_name_asm_addr, "w");

    if (*in == NULL)
    {
        printf("error: file not found\n");
        exit(1);
    }
    if (*out == NULL)
    {
        printf("error: failed to create file\n");
        exit(1);
    }
}

void tokenize(char* instruction, tokenizer* tt)
{
    char* token = strtok(instruction, " ");
    tt->command = token;
    token = strtok(NULL, " ");
    if(token != NULL)
    {
        tt->arg1 = token;
        token = strtok(NULL, " ");
        tt->arg2 = token;
    }
    else
    {
        tt->arg1 = NULL;
        tt->arg2 = NULL;
    }
}

void parse(tokenizer* tt, parser* pp)
{
    if(tt->command == NULL)
        pp->command_type = NONE;
    else if(strcmp(tt->command, "push") == 0)
        pp->command_type = PUSH;
    else if(strcmp(tt->command, "add") == 0)
        pp->command_type = ADD;
    else if(strcmp(tt->command, "sub") == 0)
        pp->command_type = SUB;
    else if(strcmp(tt->command, "neg") == 0)
        pp->command_type = NEG;
    else if(strcmp(tt->command, "eq") == 0)
        pp->command_type = EQ;
    else if(strcmp(tt->command, "gt") == 0)
        pp->command_type = GT;
    else if(strcmp(tt->command, "lt") == 0)
        pp->command_type = LT;
    else if(strcmp(tt->command, "and") == 0)
        pp->command_type = AND;
    else if(strcmp(tt->command, "or") == 0)
        pp->command_type = OR;
    else if(strcmp(tt->command, "not") == 0)
        pp->command_type = NOT;
    else if(strcmp(tt->command, "pop") == 0)
        pp->command_type = POP;
    else if(strcmp(tt->command, "label") == 0)
        pp->command_type = LABEL;
    else if(strcmp(tt->command, "if-goto") == 0)
        pp->command_type = IF_GOTO;
    else if(strcmp(tt->command, "goto") == 0)
        pp->command_type = GOTO;
    else if(strcmp(tt->command, "call") == 0)
        pp->command_type = CALL;
    else if(strcmp(tt->command, "function") == 0)
        pp->command_type = FUNCTION;
    else if(strcmp(tt->command, "return") == 0)
        pp->command_type = RETURN;

    if(tt->arg1 == NULL)
        pp->arg1_type = NONE;
    else if(strcmp(tt->arg1, "constant") == 0)
        pp->arg1_type = CONSTANT;
    else if(strcmp(tt->arg1, "local") == 0)
        pp->arg1_type = LOCAL;
    else if(strcmp(tt->arg1, "argument") == 0)
        pp->arg1_type = ARGUMENT;
    else if(strcmp(tt->arg1, "this") == 0)
        pp->arg1_type = THIS;
    else if(strcmp(tt->arg1, "that") == 0)
        pp->arg1_type = THAT;
    else if(strcmp(tt->arg1, "temp") == 0)
        pp->arg1_type = TEMP;
    else if(strcmp(tt->arg1, "static") == 0)
        pp->arg1_type = STATIC;
    else if(strcmp(tt->arg1, "pointer") == 0)
        pp->arg1_type = POINTER;
}

void write_to_file(tokenizer* tt, parser* pp, FILE** out)
{
    static int EQlabelcount = 0;
    static int GTlabelcount = 0;
    static int LTlabelcount = 0;

    switch(pp->command_type)
    {
        case ADD:
        {
            fprintf(*out, "@SP\n");
            fprintf(*out, "AM=M-1\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "A=A-1\n");
            fprintf(*out, "M=D+M\n");
        } break;

        case SUB:
        {
            fprintf(*out, "@SP\n");
            fprintf(*out, "AM=M-1\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "A=A-1\n");
            fprintf(*out, "M=M-D\n");
        } break;

        case NEG:
        {
            fprintf(*out, "@SP\n");
            fprintf(*out, "A=M-1\n");
            fprintf(*out, "M=-M\n");
        } break; 

        case EQ:
        {
            fprintf(*out, "@SP\n");
            fprintf(*out, "AM=M-1\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "A=A-1\n");
            fprintf(*out, "D=M-D\n");
            fprintf(*out, "M=-1\n");
            fprintf(*out, "@EQ%d\n", EQlabelcount);
            fprintf(*out, "D;JEQ\n");
            fprintf(*out, "@SP\n");
            fprintf(*out, "A=M-1\n");
            fprintf(*out, "M=0\n");
            fprintf(*out, "(EQ%d)\n", EQlabelcount);
            EQlabelcount++;
        } break;

        case GT:
        {
            fprintf(*out, "@SP\n");
            fprintf(*out, "AM=M-1\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "A=A-1\n");
            fprintf(*out, "D=M-D\n");
            fprintf(*out, "M=-1\n");
            fprintf(*out, "@GT%d\n", GTlabelcount);
            fprintf(*out, "D;JGT\n");
            fprintf(*out, "@SP\n");
            fprintf(*out, "A=M-1\n");
            fprintf(*out, "M=0\n");
            fprintf(*out, "(GT%d)\n", GTlabelcount);
            GTlabelcount++;
        } break;

        case LT:
        {
            fprintf(*out, "@SP\n");
            fprintf(*out, "AM=M-1\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "A=A-1\n");
            fprintf(*out, "D=M-D\n");
            fprintf(*out, "M=-1\n");
            fprintf(*out, "@LT%d\n", LTlabelcount);
            fprintf(*out, "D;JLT\n");
            fprintf(*out, "@SP\n");
            fprintf(*out, "A=M-1\n");
            fprintf(*out, "M=0\n");
            fprintf(*out, "(LT%d)\n", LTlabelcount);
            LTlabelcount++;
        } break;

        case AND:
        {
            fprintf(*out, "@SP\n");
            fprintf(*out, "AM=M-1\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "A=A-1\n");
            fprintf(*out, "M=M&D\n");
        } break;

        case OR:
        {
            fprintf(*out, "@SP\n");
            fprintf(*out, "AM=M-1\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "A=A-1\n");
            fprintf(*out, "M=M|D\n");
        } break;

        case NOT:
        {
            fprintf(*out, "@SP\n");
            fprintf(*out, "A=M-1\n");
            fprintf(*out, "M=!M\n");
        } break;

        case PUSH:
        {
            switch(pp->arg1_type)
            {
                case CONSTANT:
                {
                    fprintf(*out, "@%s\n", tt->arg2);
                    fprintf(*out, "D=A\n");
                    fprintf(*out, "@SP\n");
                    fprintf(*out, "A=M\n");
                    fprintf(*out, "M=D\n");
                    fprintf(*out, "@SP\n");
                    fprintf(*out, "M=M+1\n");
                } break;

                case LOCAL:
                case ARGUMENT:
                case THIS:
                case THAT:
                {
                    fprintf(*out, "@%s\n", tt->arg2);
                    fprintf(*out, "D=A\n");
                    if(pp->arg1_type == LOCAL)
                        fprintf(*out, "@LCL\n");
                    else if(pp->arg1_type == ARGUMENT)
                        fprintf(*out, "@ARG\n");
                    else if(pp->arg1_type == THIS)
                        fprintf(*out, "@THIS\n");
                    else if(pp->arg1_type == THAT)
                        fprintf(*out, "@THAT\n");
                    fprintf(*out, "A=D+M\n");
                    fprintf(*out, "D=M\n");
                    fprintf(*out, "@SP\n");
                    fprintf(*out, "M=M+1\n");
                    fprintf(*out, "A=M-1\n");
                    fprintf(*out, "M=D\n");
                } break;

                case TEMP:
                {
                    fprintf(*out, "@%d\n", 5 + atoi(tt->arg2));
                    fprintf(*out, "D=M\n");
                    fprintf(*out, "@SP\n");
                    fprintf(*out, "M=M+1\n");
                    fprintf(*out, "A=M-1\n");
                    fprintf(*out, "M=D\n");
                } break;

                case STATIC:
                {
                    fprintf(*out, "@%s.%s\n", file_name, tt->arg2);
                    fprintf(*out, "D=M\n");
                    fprintf(*out, "@SP\n");
                    fprintf(*out, "M=M+1\n");
                    fprintf(*out, "A=M-1\n");
                    fprintf(*out, "M=D\n");
                } break;

                case POINTER:
                {
                    if(strcmp(tt->arg2, "0") == 0)
                        fprintf(*out, "@THIS\n");
                    else if(strcmp(tt->arg2, "1") == 0)
                        fprintf(*out, "@THAT\n");
                    fprintf(*out, "D=M\n");
                    fprintf(*out, "@SP\n");
                    fprintf(*out, "M=M+1\n");
                    fprintf(*out, "A=M-1\n");
                    fprintf(*out, "M=D\n");
                } break;
            }
        } break;

        case POP:
        {
            switch(pp->arg1_type)
            {
                case LOCAL:
                case ARGUMENT:
                case THIS:
                case THAT:
                {
                    fprintf(*out, "@%s\n", tt->arg2);
                    fprintf(*out, "D=A\n");
                    if(pp->arg1_type == LOCAL)
                        fprintf(*out, "@LCL\n");
                    else if(pp->arg1_type == ARGUMENT)
                        fprintf(*out, "@ARG\n");
                    else if(pp->arg1_type == THIS)
                        fprintf(*out, "@THIS\n");
                    else if(pp->arg1_type == THAT)
                        fprintf(*out, "@THAT\n");
                    fprintf(*out, "D=D+M\n");
                    fprintf(*out, "@R13\n");
                    fprintf(*out, "M=D\n");
                    
                    fprintf(*out, "@SP\n");
                    fprintf(*out, "AM = M-1\n");
                    fprintf(*out, "D=M\n");
                    fprintf(*out, "@R13\n");
                    fprintf(*out, "A=M\n");
                    fprintf(*out, "M=D\n");
                } break;

                case TEMP:
                {
                    fprintf(*out, "@%d\n", 5 + atoi(tt->arg2));
                    fprintf(*out, "D=A\n");
                    fprintf(*out, "@R13\n");
                    fprintf(*out, "M=D\n");
                    
                    fprintf(*out, "@SP\n");
                    fprintf(*out, "AM=M-1\n");
                    fprintf(*out, "D=M\n");
                    fprintf(*out, "@R13\n");
                    fprintf(*out, "A=M\n");
                    fprintf(*out, "M=D\n");
                } break;

                case STATIC:
                {
                    fprintf(*out, "@%s.%s\n", file_name, tt->arg2);
                    fprintf(*out, "D=A\n");
                    fprintf(*out, "@R13\n");
                    fprintf(*out, "M=D\n");
                    
                    fprintf(*out, "@SP\n");
                    fprintf(*out, "AM=M-1\n");
                    fprintf(*out, "D=M\n");
                    fprintf(*out, "@R13\n");
                    fprintf(*out, "A=M\n");
                    fprintf(*out, "M=D\n");
                } break;

                case POINTER:
                {
                    fprintf(*out, "@SP\n");
                    fprintf(*out, "AM=M-1\n");
                    fprintf(*out, "D=M\n");
                    if(strcmp(tt->arg2, "0") == 0)
                        fprintf(*out, "@THIS\n");
                    else if(strcmp(tt->arg2, "1") == 0)
                        fprintf(*out, "@THAT\n");
                    fprintf(*out, "M=D\n");
                } break;

                default:
                {

                } break;
            }
        } break;

        case LABEL:
        {
            fprintf(*out, "(%s)\n", tt->arg1);
        } break;

        case IF_GOTO:
        {
            fprintf(*out, "@SP\n");
            fprintf(*out, "AM=M-1\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@%s\n", tt->arg1);
            fprintf(*out, "D;JNE\n");
        } break;

        case GOTO:
        {
            fprintf(*out, "@%s\n", tt->arg1);
            fprintf(*out, "0;JMP\n");
        } break;

        case CALL:
        {
            // save return address
            fprintf(*out, "@%s$ret.%d\n", caller_name, caller_ret_i);
            fprintf(*out, "D=A\n");
            fprintf(*out, "@SP\n");
            fprintf(*out, "M=M+1\n");
            fprintf(*out, "A=M-1\n");
            fprintf(*out, "M=D\n");

            // save old LCL
            fprintf(*out, "@LCL\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@SP\n");
            fprintf(*out, "M=M+1\n");
            fprintf(*out, "A=M-1\n");
            fprintf(*out, "M=D\n");

            //save old ARG
            fprintf(*out, "@ARG\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@SP\n");
            fprintf(*out, "M=M+1\n");
            fprintf(*out, "A=M-1\n");
            fprintf(*out, "M=D\n");

            // save old THIS 
            fprintf(*out, "@THIS\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@SP\n");
            fprintf(*out, "M=M+1\n");
            fprintf(*out, "A=M-1\n");
            fprintf(*out, "M=D\n");

            // save old THAT 
            fprintf(*out, "@THAT\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@SP\n");
            fprintf(*out, "M=M+1\n");
            fprintf(*out, "A=M-1\n");
            fprintf(*out, "M=D\n");
            
            // new LCL
            fprintf(*out, "@SP\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@LCL\n");
            fprintf(*out, "M=D\n");
            
            // new ARG
            fprintf(*out, "@%d\n", 5 + atoi(tt->arg2));
            fprintf(*out, "D=A\n");
            fprintf(*out, "@SP\n");
            fprintf(*out, "D=M-D\n");
            fprintf(*out, "@ARG\n");
            fprintf(*out, "M=D\n");

            // goto FUNCTION
            fprintf(*out, "@%s\n", tt->arg1);
            fprintf(*out, "0;JMP\n");

            // one life after the function call
            fprintf(*out, "(%s$ret.%d)\n", caller_name, caller_ret_i);

            caller_ret_i++;
        } break;

        case FUNCTION:
        {
            free(caller_name);
            caller_name = strdup(tt->arg1);
            caller_ret_i = 0;

            fprintf(*out, "(%s)\n", tt->arg1);

            int n = atoi(tt->arg2);
            for(int i = 0; i < n; i++)
            {
                fprintf(*out, "@SP\n");
                fprintf(*out, "M=M+1\n");
                fprintf(*out, "A=M-1\n");
                fprintf(*out, "M=0\n");
            }
        } break;

        case RETURN: 
        {
            // save frame end in R13
            fprintf(*out, "@LCL\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@R13\n");
            fprintf(*out, "M=D\n");

            // save return address in R14
            fprintf(*out, "@5\n");
            fprintf(*out, "D=A\n");
            fprintf(*out, "@R13\n");
            fprintf(*out, "A=M-D\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@R14\n");
            fprintf(*out, "M=D\n");

            // place return value in argument 0
            fprintf(*out, "@SP\n");
            fprintf(*out, "AM=M-1\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@ARG\n");
            fprintf(*out, "A=M\n");
            fprintf(*out, "M=D\n");

            // New SP location
            fprintf(*out, "@ARG\n");
            fprintf(*out, "D=M+1\n");
            fprintf(*out, "@SP\n");
            fprintf(*out, "M=D\n");

            // old THAT
            fprintf(*out, "@1\n");
            fprintf(*out, "D=A\n");
            fprintf(*out, "@R13\n");
            fprintf(*out, "A=M-D\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@THAT\n");
            fprintf(*out, "M=D\n");

            // old THIS
            fprintf(*out, "@2\n");
            fprintf(*out, "D=A\n");
            fprintf(*out, "@R13\n");
            fprintf(*out, "A=M-D\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@THIS\n");
            fprintf(*out, "M=D\n");

            // old ARG
            fprintf(*out, "@3\n");
            fprintf(*out, "D=A\n");
            fprintf(*out, "@R13\n");
            fprintf(*out, "A=M-D\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@ARG\n");
            fprintf(*out, "M=D\n");

            // old LCL
            fprintf(*out, "@4\n");
            fprintf(*out, "D=A\n");
            fprintf(*out, "@R13\n");
            fprintf(*out, "A=M-D\n");
            fprintf(*out, "D=M\n");
            fprintf(*out, "@LCL\n");
            fprintf(*out, "M=D\n");
            
            // jump to parent caller frame
            fprintf(*out, "@R14\n");
            fprintf(*out, "A=M\n");
            fprintf(*out, "0;JMP\n");
        } break;
    }
}

void translate(FILE **in, FILE **out)
{
    char buffer_in[256];

    while (fgets(buffer_in, 256, *in) != NULL)
    {
        char instruction[101];
        strcpy(instruction, buffer_in); 
        instruction[strlen(instruction) - 1] = '\0'; // when testing it out make it -2

        if(instruction[0] != '/' && instruction[0] != '\0')
        {
            fprintf(*out, "//%s\n", instruction);

            tokenizer tt = {0};
            parser pp = {0};

            tokenize(instruction, &tt);

            parse(&tt, &pp);

            write_to_file(&tt, &pp, out);
        }
    }
}

int main(int argc, char *argv[])
{
    FILE *in;
    FILE *out;
    DIR *dir;

    caller_name = strdup("ProgMainSysStartInit");
    caller_ret_i = 0;

    char* arg1 = *(argv+1);
    if(arg1[strlen(arg1) - 1] == '/')
        arg1[strlen(arg1) - 1] = '\0';

    if(strstr(arg1, ".vm") != NULL)
    {
        // vm file

        char file_without_extension[501];
        sscanf(arg1, "%[^.]", file_without_extension);

        char* slash = strrchr(file_without_extension, '/');
        if(slash != NULL)
            file_name = slash + 1; 
        else
            file_name = file_without_extension;

        char vm_in[501];
        strcpy(vm_in, file_without_extension);
        strcat(vm_in, ".vm");
        in = fopen(vm_in, "r");
        if(in == NULL){printf("vm file not found\n"); exit(1);}

        char asm_out[501];
        strcpy(asm_out, file_without_extension);
        strcat(asm_out, ".asm");
        out = fopen(asm_out, "w");
        if(out == NULL){printf("failed to create asm file\n"); exit(1);}

        translate(&in, &out); 

        fprintf(out, "(END)\n");
        fprintf(out, "@END\n");
        fprintf(out, "0;JMP\n");

        fclose(in);
        fclose(out);
    }
    else
    {
        // directory

        dir = opendir(arg1);
        if (dir == NULL) {printf("directory not found\n");exit(1);}

        char asm_out[501];
        strcpy(asm_out, arg1);
        char* folder = strrchr(arg1, '/');
        if(folder != NULL)
            strcat(asm_out, folder);
        else
        {
            strcat(asm_out, "/");
            strcat(asm_out, arg1);
        }
        strcat(asm_out, ".asm");
        out = fopen(asm_out, "w");
        if(out == NULL){printf("failed to create asm file\n"); exit(1);}      

        // bootstrap
        fprintf(out, "//bootstrap code\n");
        fprintf(out, "//SP=256\n");
        fprintf(out, "@256\n");
        fprintf(out, "D=A\n");
        fprintf(out, "@SP\n");
        fprintf(out, "M=D\n");

        char callsysinit[] = "call Sys.init 0";
        fprintf(out, "//%s\n", callsysinit);
        tokenizer tt = {0};
        parser pp = {0};
        tokenize(callsysinit, &tt);
        parse(&tt, &pp);
        write_to_file(&tt, &pp, &out);
        fprintf(out, "//\n");
        //

        struct dirent* entity;
        entity = readdir(dir);
        while (entity != NULL) {
            if (strstr(entity->d_name, ".vm") != NULL)
            {
                // printf("translating %s\n", entity->d_name);

                char file_without_extension[501];
                sscanf(entity->d_name, "%[^.]", file_without_extension);
                file_name = file_without_extension;

                char vm_in[501];
                strcpy(vm_in, arg1);
                strcat(vm_in, "/");
                strcat(vm_in, entity->d_name);
                in = fopen(vm_in, "r");
                if(in == NULL){printf("vm file not found\n"); exit(1);} 

                translate(&in, &out);

                fclose(in);
            }
            entity = readdir(dir);
        }

        fprintf(out, "(END)\n");
        fprintf(out, "@END\n");
        fprintf(out, "0;JMP\n");
        
        closedir(dir);
        fclose(out);
    }

    return 0;
}
