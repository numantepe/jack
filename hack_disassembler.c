#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define HASH_TABLE_SIZE 101
typedef int32_t bool32;

typedef struct nlist
{
    struct nlist *next;
    char *name;
    char *defn;
} nlist;

typedef struct hash_table
{
    nlist *arr[HASH_TABLE_SIZE];
} hash_table;

unsigned hash(char *s)
{
    unsigned hashval;

    for (hashval = 0; *s != '\0'; s++)
    {
        hashval = *s + 31 * hashval;
    }

    return hashval % HASH_TABLE_SIZE;
}

nlist *lookup(hash_table *ht, char *s)
{
    nlist *np;

    for (np = ht->arr[hash(s)]; np != NULL; np = np->next)
    {
        if (strcmp(s, np->name) == 0)
            return np;
    }

    return NULL;
}

nlist *install(hash_table *ht, char *name, char *defn)
{
    nlist *np;
    unsigned hashval;

    if ((np = lookup(ht, name)) == NULL)
    {
        np = (nlist *)malloc(sizeof(*np));
        np->name = strdup(name);
        np->defn = strdup(defn);
        hashval = hash(name);
        np->next = ht->arr[hashval];
        ht->arr[hashval] = np;
    }
    else
    {
        free((void *)np->defn);
        np->defn = strdup(defn);
    }

    return np;
}

typedef struct parser
{
    char dest[4];
    char comp[4];
    char jump[4];
} parser;

typedef struct code
{
    char comp[8];
    char dest[4];
    char jump[4];
} code;

static hash_table symbol_table;
static hash_table comp_ref;
static char dest_ref[][4] = {"", "M", "D", "MD", "A", "AM", "AD", "AMD"};
static char jump_ref[][4] = {"", "JGT", "JEQ", "JGE", "JLT", "JNE", "JLE", "JMP"};
static char binary_rep[][4] = {"000", "001", "010", "011", "100", "101", "110", "111"};

void setup_hash_tables()
{
    install(&symbol_table, "SP", "0");
    install(&symbol_table, "LCL", "1");
    install(&symbol_table, "ARG", "2");
    install(&symbol_table, "THIS", "3");
    install(&symbol_table, "THAT", "4");
    install(&symbol_table, "SCREEN", "16384");
    install(&symbol_table, "KBD", "24576");
    install(&symbol_table, "R0", "0");
    install(&symbol_table, "R1", "1");
    install(&symbol_table, "R2", "2");
    install(&symbol_table, "R3", "3");
    install(&symbol_table, "R4", "4");
    install(&symbol_table, "R5", "5");
    install(&symbol_table, "R6", "6");
    install(&symbol_table, "R7", "7");
    install(&symbol_table, "R8", "8");
    install(&symbol_table, "R9", "9");
    install(&symbol_table, "R10", "10");
    install(&symbol_table, "R11", "11");
    install(&symbol_table, "R12", "12");
    install(&symbol_table, "R13", "13");
    install(&symbol_table, "R14", "14");
    install(&symbol_table, "R15", "15");

    install(&comp_ref, "0", "0101010");
    install(&comp_ref, "1", "0111111");
    install(&comp_ref, "-1", "0111010");
    install(&comp_ref, "D", "0001100");
    install(&comp_ref, "A", "0110000");
    install(&comp_ref, "!D", "0001101");
    install(&comp_ref, "!A", "0110001");
    install(&comp_ref, "-D", "0001111");
    install(&comp_ref, "-A", "0110011");
    install(&comp_ref, "D+1", "0011111");
    install(&comp_ref, "A+1", "0110111");
    install(&comp_ref, "D-1", "0001110");
    install(&comp_ref, "A-1", "0110010");
    install(&comp_ref, "D+A", "0000010");
    install(&comp_ref, "D-A", "0010011");
    install(&comp_ref, "A-D", "0000111");
    install(&comp_ref, "D&A", "0000000");
    install(&comp_ref, "D|A", "0010101");
    install(&comp_ref, "M", "1110000");
    install(&comp_ref, "!M", "1110001");
    install(&comp_ref, "-M", "1110011");
    install(&comp_ref, "M+1", "1110111");
    install(&comp_ref, "M-1", "1110010");
    install(&comp_ref, "D+M", "1000010");
    install(&comp_ref, "D-M", "1010011");
    install(&comp_ref, "M-D", "1000111");
    install(&comp_ref, "D&M", "1000000");
    install(&comp_ref, "M&D", "1000000");
    install(&comp_ref, "D|M", "1010101");
    install(&comp_ref, "M|D", "1010101");
}

int main(int argc, char *argv[])
{
    char file_name[101];
    sscanf(*(argv + 1), "%[^.]", file_name);

    char file_name_asm_addr[101];
    strcpy(file_name_asm_addr, file_name);
    strcat(file_name_asm_addr, ".asm");

    char file_name_hack_addr[101];
    strcpy(file_name_hack_addr, file_name);
    strcat(file_name_hack_addr, ".hack");
    // printf("translating %s\n", file_name_asm_addr);

    setup_hash_tables();

    FILE *in;
    FILE *out;

    char buffer_in[256], buffer_out[256];

    in = fopen(file_name_asm_addr, "r");
    out = fopen(file_name_hack_addr, "w");

    if (in == NULL)
    {
        printf("error: file not found\n");
        return 1;
    }
    if (out == NULL)
    {
        printf("error: failed to create file\n");
        return 1;
    }

    // while - labels
    int line_num_int = 0;

    while (fgets(buffer_in, 256, in) != NULL)
    {
        char instruction[101];
        sscanf(buffer_in, "%s", instruction);

        if (instruction[0] == '(')
        {
            char label[99];
            sscanf(instruction, "(%[^)]", label);
            char line_num_str[11];
            sprintf(line_num_str, "%d", line_num_int);
            if (lookup(&symbol_table, label) == NULL)
                install(&symbol_table, label, line_num_str);
        }

        if (instruction[0] != '/' && instruction[0] != '\0' && instruction[0] != '(')
            line_num_int++;
    }

    rewind(in);

    // while - Variables
    int var_addr_int = 16;

    while (fgets(buffer_in, 256, in) != NULL)
    {
        char instruction[101];
        sscanf(buffer_in, "%s", instruction);

        if (instruction[0] == '@')
        {
            char ARegister_new_value_str[100];
            sscanf(instruction, "@%s", ARegister_new_value_str);

            if (isdigit(ARegister_new_value_str[0]) == 0 && lookup(&symbol_table, ARegister_new_value_str) == NULL)
            {
                char var_addr_str[11];
                sprintf(var_addr_str, "%d", var_addr_int);
                install(&symbol_table, ARegister_new_value_str, var_addr_str);
                var_addr_int++;
            }
        }
    }

    rewind(in);

    while (fgets(buffer_in, 256, in) != NULL)
    {
        parser pp;
        code cc;

        char instruction[101];
        sscanf(buffer_in, "%s", instruction);

        if (instruction[0] != '/' && instruction[0] != '\0' && instruction[0] != '(') // TODO(numan): and later && instruction[0] != '(' as well
        {
            if (strchr(instruction, '@') != NULL)
            {
                strcpy(buffer_out, "0000000000000000");
                char LVN[100];
                sscanf(instruction, "@%s", LVN);

                char ARegister_new_value_str[11];

                if (lookup(&symbol_table, LVN) != NULL)
                    strcpy(ARegister_new_value_str, lookup(&symbol_table, LVN)->defn);
                else
                    strcpy(ARegister_new_value_str, LVN);

                int ARegister_new_value_int = atoi(ARegister_new_value_str);

                int i, rem, quo;

                i = 15;
                quo = ARegister_new_value_int;

                while (quo != 0)
                {
                    rem = quo % 2;

                    if (rem == 0)
                        buffer_out[i] = '0';
                    else
                        buffer_out[i] = '1';

                    quo = quo / 2;
                    i--;
                }
            }
            else
            {
                if (strchr(instruction, '=') != NULL)
                {
                    sscanf(instruction, "%[^=]=%s", pp.dest, pp.comp);

                    strcpy(cc.jump, "000");
                    strcpy(cc.comp, lookup(&comp_ref, pp.comp)->defn);
                    int i;
                    for (i = 0; i < 8; i++)
                        if (strcmp(pp.dest, dest_ref[i]) == 0)
                            break;
                    strcpy(cc.dest, binary_rep[i]);
                }
                else if (strchr(instruction, ';') != NULL)
                {
                    sscanf(instruction, "%[^;];%s", pp.comp, pp.jump);

                    strcpy(cc.dest, "000");
                    strcpy(cc.comp, lookup(&comp_ref, pp.comp)->defn);
                    int i;
                    for (i = 0; i < 8; i++)
                        if (strcmp(pp.jump, jump_ref[i]) == 0)
                            break;
                    strcpy(cc.jump, binary_rep[i]);
                }
                else
                {
                }

                sprintf(buffer_out, "111%s%s%s", cc.comp, cc.dest, cc.jump);
            }

            fwrite(buffer_out, sizeof(char), strlen(buffer_out), out);
            fprintf(out, "\n");
        }
    }

    fclose(in);
    fclose(out);
    return 0;
}
