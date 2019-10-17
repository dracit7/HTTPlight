#include <stdio.h>
#include <assert.h>

#include "lib/socket.h"

int main() {

  printf("Started testing...\n");

  assert(set_listen_addr("127.0.0.1") == 0);
  assert(set_listen_port(9290) == 0);
  assert(set_listen_port(-2) < 0);
  assert(set_listen_port(92837) < 0);
  assert(set_fs_path("/etc") == 0);
  assert(set_fs_path("/abc") < 0);

  printf("Configuration functions checked.\n");

  set_listen_addr("127.0.0.1");
  set_listen_port(14514);
  set_fs_path("/tmp");
  printf("init_server: %d\n", init_server());

  return 0;

}