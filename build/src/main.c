#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

typedef struct _ARGS_T {
	uint8_t run;
	char** runWith;
	uint16_t runWithLen;
	uint8_t all;
} args_t;

args_t* parse_args(int argc, char** argv);
void run(args_t* args);
char** get_modules(size_t* len);
void add_module(char* module);
void remove_module(char* module);
int contains(char** array, size_t len, char* key);

int main(int argc, char** argv) {
    if (system(NULL) == 0) {
        printf("Error! No shell available!\n");
        exit(1);
    }
	args_t* args = parse_args(argc, argv);
	if (args->run == 0) return 0;
	run(args);
}

args_t* parse_args(int argc, char** argv) {
	args_t* args = calloc(1, sizeof(struct _ARGS_T));

	if (argc < 2) {
		printf("Error: Not enough arguments given!\n");
		exit(1);
	}

	if (strcmp(argv[1], "run") == 0) {
		args->run = 1;

		if (argc < 3) {
			args->all = 1;
            args->runWithLen = 0;
			return args;
		}

		if (strcmp(argv[2], "none") == 0) {
			args->all = 0;
            args->runWithLen = 0;
			return args;
		}
		else if (strcmp(argv[2], "add") == 0) {
			args->all = 0;
			int counter = 4;
			args->runWith = calloc(1, sizeof(char*));
			while (argc >= counter) {
				args->runWithLen++;
				args->runWith = realloc(args->runWith, args->runWithLen * sizeof(char*));
				args->runWith[args->runWithLen - 1] = argv[counter - 1];
				counter++;
			}
		}
		else if (strcmp(argv[2], "remove") == 0) {
			args->all = 1;
			int counter = 4;
			args->runWith = calloc(1, sizeof(char*));
			while (argc >= counter) {
				args->runWithLen++;
				args->runWith = realloc(args->runWith, args->runWithLen * sizeof(char*));
				args->runWith[args->runWithLen - 1] = argv[counter - 1];
				counter++;
			}
		}
		else {
			printf("Error: Invalid argument!\n");
			exit(1);
		}

	}
	else if (strcmp(argv[1], "edit") == 0) {
		args->run = 0;

		if (argc < 3) {
			printf("Error: Not enough arguments given!\n");
			exit(1);
		}

        char* cmd = calloc(17 + strlen(argv[2]) + 5, sizeof(char));

        add_module(argv[2]);

		strcpy(cmd, "touch hooks/src/");
		strcat(cmd, argv[2]);
		strcat(cmd, ".cpp");

		system(cmd);

		cmd = calloc(16 + strlen(argv[2]) + 5, sizeof(char));

		strcpy(cmd, "nvim hooks/src/");
		strcat(cmd, argv[2]);
		strcat(cmd, ".cpp");

		system(cmd);
	}
	else if (strcmp(argv[1], "remove") == 0) {
        if (argc < 3) {
            printf("ERROR! Not enough arguments given!\n");
            exit(1);
        }

        char* cmd = calloc(14 + strlen(argv[2]) + 5, sizeof(char));

        add_module(argv[2]);

		strcpy(cmd, "rm hooks/src/");
		strcat(cmd, argv[2]);
		strcat(cmd, ".cpp");

		system(cmd);

        remove_module(argv[2]);
	}
    else if (strcmp(argv[1], "list") == 0) {
        args->run = 0;
        size_t len = 0;
        char** list = get_modules(&len);
        printf("Listing all modules:\n");
        for (size_t i = 0; i < len; i++) {
            printf("- %s\n", list[i]);
        }
    }
	else {
		printf("Error: Invalid argument!\n");
		exit(1);
	}

	return args;
}

void run(args_t* args) {
    FILE* runFile = fopen("run.sh", "w");

    fprintf(runFile, "cd hooks && make && cd ..\n");

    char* path = realpath("hooks/bin/", NULL);

    char* ldpp = calloc(15 + strlen(path), sizeof(char));

    strcpy(ldpp, "export LDPP=");
    strcat(ldpp, path);

    fprintf(runFile, "%s\n", ldpp);

    if (args->all == 0) {
        if (args->runWithLen > 0) {
            char* cmd = calloc(19, sizeof(char));
            size_t len;
            strcpy(cmd, "export LD_PRELOAD=");
            char** modules = get_modules(&len);
            for (size_t i = 0; i < len; i++) {
                if (contains(args->runWith, args->runWithLen, strtok(modules[i], "\n")) == 1) {
                    cmd = realloc(cmd, (strlen(cmd) + strlen(modules[i]) + 12) * sizeof(char));
                    strcat(cmd, "$LDPP/");
                    strcat(cmd, strtok(modules[i], "\n"));
                    strcat(cmd, ".so:");
                }
            }
            fprintf(runFile, "%s\n", cmd);
        }
        else {
            fprintf(runFile, "export LD_PRELOAD=\n");
        }
    }
    else if (args->all == 1) {
        if (args->runWithLen > 0) {
            char* cmd = calloc(19, sizeof(char));
            size_t len;
            strcpy(cmd, "export LD_PRELOAD=");
            char** modules = get_modules(&len);
            for (size_t i = 0; i < len; i++) {
                if (contains(args->runWith, args->runWithLen, strtok(modules[i], "\n")) == 0) {
                    cmd = realloc(cmd, (strlen(cmd) + strlen(modules[i]) + 12) * sizeof(char));
                    strcat(cmd, "$LDPP/");
                    strcat(cmd, strtok(modules[i], "\n"));
                    strcat(cmd, ".so:");
                }
            }
            fprintf(runFile, "%s\n", cmd);
        }
        else if (args->runWithLen == 0) {
            char* cmd = calloc(19, sizeof(char));
            size_t len;
            strcpy(cmd, "export LD_PRELOAD=");
            char** modules = get_modules(&len);
            for (size_t i = 0; i < len; i++) {
                cmd = realloc(cmd, (strlen(cmd) + strlen(modules[i]) + 20) * sizeof(char));
                strcat(cmd, "$LDPP/");
                strcat(cmd, strtok(modules[i], "\n"));
                strcat(cmd, ".so:");
            }
            fprintf(runFile, "%s\n", cmd);
        }
        else {
            printf("Error! Something went wrong when parsing modules!\n");
            exit(1);
        }
    }
    else {
        printf("Error! Something went wrong trying to parse command arguments!\n");
        exit(1);
    }
	fprintf(runFile, "./PwnAdventure3-Linux-Shipping");
    fclose(runFile);

    system("chmod +x run.sh");
    system("./run.sh");
    system("rm run.sh");
}

char** get_modules(size_t* len) {
	FILE* modules = fopen("build/modules.dat", "rb");

    char* line = NULL;
    ssize_t read;
    char** buffer = calloc(1, sizeof(char*));
    size_t llen = 0;
    *len = 0;

    while ((read = getline(&line, &llen, modules)) != -1) {
        (*len)++;
        buffer = realloc(buffer, (*len) * sizeof(char*));
        buffer[(*len) - 1] = calloc(strlen(line) + 1, sizeof(char));
        strcpy(buffer[(*len) - 1], line);
    }

	fclose(modules);

    if (line) {
        free(line);
    }

	return buffer;
}

void add_module(char* module) {

    size_t len;
	char** modules = get_modules(&len);

    for (size_t i = 0; i < len; i++) {
        if (strcmp(module, strtok(modules[i], "\n")) == 0) return;
    }

    FILE* modFile = fopen("build/modules.dat", "a");
    fprintf(modFile, "%s", module);
    fprintf(modFile, "\n");
    fclose(modFile);

    printf("file write\n");
}

void remove_module(char* module) {
    size_t len;
	char** modules = get_modules(&len);

    char* newModules = calloc(1, sizeof(char));

    for (size_t i = 0; i < len; i++) {
        if (strcmp(module, strtok(modules[i], "\n")) != 0) {
            newModules = realloc(newModules, (strlen(newModules) + strlen(modules[i]) + 2) * sizeof(char));
            strcat(newModules, modules[i]);
        }
    }

    FILE* modFile = fopen("build/modules.dat", "w");
    fprintf(modFile, "%s", newModules);
}

int contains(char** array, size_t len, char* key) {
    for (size_t i = 0; i < len; i++) {
        if (strcmp(array[i], key) == 0) return 1;
    }
    return 0;
}
