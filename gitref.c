#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// quick hack to print ref name of current git
// repository (if any) with desired prefix
// and suffix. useful for shell prompts.
//
// you probably want to use "git rev-parse" or
// "git symbolic-ref" instead :)

const int REF_LENGTH = 7; // truncate git commit hashes to this length

char* prefix;
char* suffix;

void fail(char *errmsg) {
  fprintf(stderr, "gitref: error: %s\n", errmsg);
  exit(1);
}

void print_ref(char* ref) {
  if(prefix != NULL)
    fputs(prefix, stdout);

  fputs(ref, stdout);

  if(suffix != NULL)
    fputs(suffix, stdout);
}

int try_git_dir(char* dir) {
  char head_file[PATH_MAX];

  snprintf(head_file, sizeof(head_file), "%s%s", dir, "/.git/HEAD");

  FILE* f = fopen(head_file, "r");

  if(f == NULL)
    return 1; // failed to open HEAD file

  char ref[256];

  if(fgets(ref, sizeof(ref), f) == NULL)
    fail("failed to read ref from HEAD file");

  strtok(ref, "\n"); // trim trailing newline

  if(fclose(f) != 0)
    fail("failed to close HEAD file");

  char* last_slash = rindex(ref, '/');

  if(last_slash == 0) {
    // no slash found -- detached HEAD state
    char out[REF_LENGTH];
    strncpy(out, ref, sizeof(out));
    print_ref(out);
  } else {
    // print branch name only
    print_ref(last_slash+1);
  }

  return 0; // success
}

int main(int argc, char **argv) {
  if(argc > 1)
    prefix = argv[1];
  if(argc > 2)
    suffix = argv[2];

  char dir[PATH_MAX];

  if(getcwd(dir, sizeof(dir)) == NULL)
    fail("could not get current working directory");

  while(try_git_dir(dir)) {
    if(strlen(dir) == 1) // root of file system already reached
      return 1;
    dirname(dir); // go up one level
  }

  return 0; // successfully found git directory
}
