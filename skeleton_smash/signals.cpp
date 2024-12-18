#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num)
{
  // TODO: Add your implementation
  std::cout << "smash: got ctrl-C" << std::endl;
  pid_t pid = SmallShell::getInstance().curr_pid;
  if (pid)
  {

    if ((kill(pid, 9)) == -1)
    {
      perror("smash error: kill failed");
    }

    std::cout << "smash: process " << pid << " was killed" << std::endl;
  }
}

void alarmHandler(int sig_num)
{
  // TODO: Add your implementation
}
