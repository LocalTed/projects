#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <errno.h>
#include "Commands.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

using namespace std;

const string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == string::npos) ? "" : s.substr(start);
}

string _rtrim(const string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const string &s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args)
{
  FUNC_ENTRY()
  int i = 0;
  istringstream iss(_trim(string(cmd_line)).c_str());
  for (string s; iss >> s;)
  {
    args[i] = (char *)malloc(s.length() + 1);
    memset(args[i], 0, s.length() + 1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}
// ----------------------------------------------------------------------------------------------------------------------------------

char *replace_symb(const char *cmd_line, char c, char replacement)
{
  char *new_cmd = (char *)malloc(80 * sizeof(char));
  for (int i = 0; i < 80; i++)
  {
    if (cmd_line[i] == '\0')
    {
      break;
    }
    else if (cmd_line[i] == c)
    {
      new_cmd[i] = replacement;
    }

    else
    {
      new_cmd[i] = cmd_line[i];
    }
  }
  return new_cmd;
}
// ----------------------------------------------------------------------------------------------------------------------------------
// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell()
{
  // TODO: add your implementation
  this->m_jobs = new JobsList();
  this->io_flag = false;
  this->m_stdout = 1;
  // this->m_stderr = 2;
  this->curr_pid = 0;
}

SmallShell::~SmallShell()
{
  delete m_jobs;
}

string SmallShell::getName()
{
  return this->smashName;
}

void SmallShell::retrieve_io()
{
  if (this->io_flag)
  {
    if (dup2(this->m_stdout, 1) == -1)
    {
      perror("smash error: dup2 failed");
    }
    if (close(this->m_stdout) == -1)
    {
      perror("smash error: close failed");
    }
  }
  this->io_flag = false;
}

/////////////////////////////////// JOBS /////////////////////////////////////
JobsList::JobsList()
{
  this->m_jobs_list = new vector<JobEntry *>();
}

JobsList::~JobsList()
{
  delete m_jobs_list;
}

void JobsList::addJob(Command *cmd, pid_t pid)
{
  this->removeFinishedJobs();
  int id = this->m_jobs_list->empty() ? 1 : (this->m_jobs_list->back()->get_id() + 1);
  this->m_jobs_list->push_back(new JobEntry(id, pid, cmd->get_name()));
}

void JobsList::printJobsList()
{
  this->removeFinishedJobs();
  for (vector<JobEntry *>::iterator curr = this->m_jobs_list->begin(); curr < this->m_jobs_list->end(); curr++)
  {
    cout << '[' << (*curr)->get_id() << ']' << ' ' << (*curr)->get_name() << endl;
  }
}

void JobsList::removeFinishedJobs()
{

  // cout << "RFJ called. #jobs: " << m_jobs_list->size() << endl;
  int res, status;
  for (auto curr = this->m_jobs_list->begin(); curr != this->m_jobs_list->end();)
  {
    pid_t curr_pid = (*curr)->get_pid();

    res = waitpid(curr_pid, &status, WNOHANG);
    // cout << "\t------------------------------ waiting here" << endl;
    // cout << "\tAt job pid: " << curr_pid << endl;
    // cout << "\twaitpid NOHANG res: " << res << endl;
    // cout << "\tchild status: " << status << endl;

    if (res == -1 && errno != ECHILD)
    {
      // if ()
      // {
      //   cout << "ECHILD CAUGHT" << endl;
      //   curr = this->m_jobs_list->erase(curr);
      // }
      // else
      // {
      perror("smash error: waitpid failed");
      return;
      // }
    }
    else if (res == curr_pid)
    {
      // waitpid((*curr)->get_pid(), NULL, 0);
      // cout << "COUGHT FINISHED PID: " << res << endl;
      curr = this->m_jobs_list->erase(curr);
    }
    else
    {
      ++curr;
    }
  }
}

void JobsList::killAllJobs()
{
  // this->removeFinishedJobs();
  for (vector<JobEntry *>::iterator curr = this->m_jobs_list->begin(); curr < this->m_jobs_list->end(); curr++)
  {
    cout << (*curr)->get_pid() << ": " << (*curr)->get_name() << endl;
    if (kill((*curr)->get_pid(), SIGKILL) == -1) // SEGEL TEST FAILS HERE:
    {
      perror("smash error: kill failed");
    }
  }
  this->m_jobs_list->clear(); // std::vector->clear() ?
}

int JobsList::get_jobs_num()
{
  return this->m_jobs_list->size();
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  if (jobId <= 0 || this->m_jobs_list->empty())
    return nullptr;
  vector<JobEntry *>::iterator curr = this->m_jobs_list->begin();
  while (*curr != nullptr)
  {
    if ((*curr)->get_id() == jobId)
      return *curr;
    curr++;
  }
  return nullptr;
}

void JobsList::removeJobById(int jobId)
{
  for (auto curr = this->m_jobs_list->begin(); curr != this->m_jobs_list->end(); ++curr)
  {
    if ((*curr)->get_id() == jobId)
    {
      this->m_jobs_list->erase(curr);
      return;
    }
  }
}

char *JobsList::getLastJobName()
{
  return this->m_jobs_list->back()->get_name();
}

pid_t JobsList::removeLastJob()
{
  pid_t pid = this->m_jobs_list->back()->get_pid();
  this->m_jobs_list->pop_back();
  return pid;
}

///////////////////////////////////// JOB ENTRY /////////////////////////////////////
JobsList::JobEntry::JobEntry(int jobid, pid_t pid, const char *name) : jobid(jobid), pid(pid)
{
  this->job_name = (char *)malloc(80 * sizeof(char));
  strcpy(this->job_name, name);
}

int JobsList::JobEntry::get_id()
{
  return this->jobid;
}

char *JobsList::JobEntry::get_name()
{
  return this->job_name;
}

pid_t JobsList::JobEntry::get_pid()
{
  return this->pid;
}

////////////////////////////////////////// COMMANDS /////////////////////////////////////
Command::Command(const char *cmd_line) : cmd_line(cmd_line), old_unstripped(cmd_line)
{

  this->args = (char **)malloc(20 * sizeof(char *));
  this->argsnum = _parseCommandLine(cmd_line, args);
}

Command::~Command()
{
  free(args);
}

const char *Command::get_name()
{
  return this->old_unstripped;
}

void Command::shorten_args()
{
  // remove last args
  argsnum -= 2;
  char **new_args = (char **)malloc(argsnum * sizeof(char *));
  for (int i = 0; i < argsnum; i++)
  {
    new_args[i] = this->args[i];
  }

  free(this->args);
  this->args = new_args;
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line)
{
  if (_isBackgroundComamnd(cmd_line))
  {
    char *new_cmd = (char *)malloc(80 * sizeof(char));
    strcpy(new_cmd, cmd_line);
    _removeBackgroundSign(new_cmd);
    this->argsnum = _parseCommandLine(new_cmd, args);
    this->cmd_line = new_cmd;
  }
}

int BuiltInCommand::get_int_arg(char *s, bool *success, int base)
{
  char **strend = &s;
  int id = (int)strtol(s, strend, base);
  if (**strend == '\0')
  {
    *success = true;
    return id;
  }
  *success = false;
  return 0;
}

/////////////////////////////////////// EXTERNAL COMMANDS /////////////////////////////////////
ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line)
{
  this->is_bg = false;
  // check if bg and strip &
  if (_isBackgroundComamnd(cmd_line))
  {
    char *new_cmd = (char *)malloc(80 * sizeof(char));
    this->is_bg = true;
    strcpy(new_cmd, cmd_line);
    _removeBackgroundSign(new_cmd);
    this->argsnum = _parseCommandLine(new_cmd, args);
    this->cmd_line = new_cmd;
  }
}

ExternalCommand::~ExternalCommand() {}

bool ExternalCommand::is_complex_ext()
{
  if (this->cmd_line == NULL)
    return false;
  char star = '*', question = '?';
  const char *ptr = this->cmd_line;

  while (*ptr != '\0')
  {
    if (*ptr == star || *ptr == question)
    {
      return true;
    }
    ptr++;
  }
  return false;
}

void ExternalCommand::execute()
{
  char *prefix = (char *)malloc(86 * sizeof(char));
  strcpy(prefix, "/bin/");
  strcat(prefix, args[0]);
  // prefix contains path: /bin/cmd

  pid_t pid = fork();

  if (pid == 0) // is child
  {
    if (setpgrp() == -1)
    {
      perror("smash error: setpgrp failed");
      exit(0);
    }

    // bash
    if (this->is_complex_ext())
    {
      char **new_args = (char **)malloc(4 * sizeof(char *));
      new_args[3] = NULL;
      new_args[2] = (char *)malloc(80 * sizeof(char));
      new_args[1] = (char *)"-c";
      strcpy(new_args[2], this->cmd_line);
      new_args[0] = (char *)"bash"; //(char *)"/bin/bash\0";

      if (execvp(new_args[0], new_args) == -1)
      {
        perror("smash error: execvp failed");
        exit(0);
      }
    }

    // not bash
    else
    {
      if (execvp(args[0], args) == -1)
      {
        perror("smash error: execvp failed");
        exit(0);
      }
    }
    exit(0);
  }

  else if (pid == -1)
  {
    perror("smash error: fork failed");
  }

  else if (this->is_bg) // background cmd
  {
    // should not wait to child, should add to JobsList and let it run, Forrest, run
    SmallShell::getInstance().m_jobs->addJob(this, pid);
  }
  else
  {
    SmallShell::getInstance().curr_pid = pid;
    if (waitpid(pid, NULL, 0) == -1)
    {
      perror("smash error: waitpid failed");
    }
    SmallShell::getInstance().curr_pid = 0;

  } // no backgroung
}

/////////////////////////////////////// FACTORY: /////////////////////////////////////
/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line)
{

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  // todo: check it's correct variable - if not check in str(cmd_line)
  if (cmd_s.find_first_of('>') != string::npos)
    return new RedirectionCommand(cmd_line);

  size_t pos;
  if ((pos = cmd_s.find_first_of('|')) != string::npos)
    return new PipeCommand(cmd_line, (int)pos);

  if (firstWord.compare("pwd") == 0 || firstWord.compare("pwd&") == 0)
  {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0 || firstWord.compare("showpid&") == 0)
  {
    return new ShowPidCommand(cmd_line);
  }
  else if (firstWord.compare("chprompt") == 0 || firstWord.compare("chprompt&") == 0)
  {
    return new RenameSmashCommand(cmd_line);
  }
  else if (firstWord.compare("cd") == 0 || firstWord.compare("cd&") == 0)
  {
    return new ChangeDirCommand(cmd_line, &this->lastPwd);
  }
  else if (firstWord.compare("jobs") == 0 || firstWord.compare("jobs&") == 0)
  {
    return new JobsCommand(cmd_line, this->m_jobs);
  }
  else if (firstWord.compare("quit") == 0 || firstWord.compare("quit&") == 0)
  {
    return new QuitCommand(cmd_line, this->m_jobs);
  }
  else if (firstWord.compare("fg") == 0 || firstWord.compare("fg&") == 0)
  {
    return new ForegroundCommand(cmd_line, this->m_jobs);
  }
  else if (firstWord.compare("kill") == 0 || firstWord.compare("kill&") == 0)
  {
    return new KillCommand(cmd_line, this->m_jobs);
  }
  else if (firstWord.compare("chmod") == 0 || firstWord.compare("chmod&") == 0)
  {
    return new ChmodCommand(cmd_line);
  }

  else
  {
    return new ExternalCommand(cmd_line);
  }

  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{

  // if (cmd_line == nullptr || *cmd_line == '\n')
  if (cmd_line == nullptr || _ltrim(cmd_line) == "")
    return;
  Command *cmd = CreateCommand(cmd_line);
  if (cmd == nullptr)
    return;
  this->m_jobs->removeFinishedJobs();
  cmd->execute();

  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void SmallShell::setSmashName(string name)
{
  smashName = name;
}

/////////////////////////////////////// PWD COMMAND /////////////////////////////////////
void GetCurrDirCommand::execute()
{
  char *path = getcwd(NULL, 0);
  if (path == nullptr)
    perror("smash error: getcwd failed");

  else
    cout << path << endl;
}

/////////////////////////////////////// PID COMMAND /////////////////////////////////////
void ShowPidCommand::execute()
{
  cout << "smash pid is " << getpid() << endl;
}

/////////////////////////////////////// CHPROMPT COMMAND /////////////////////////////////////
void RenameSmashCommand::execute()
{
  if (argsnum == 1)
  {
    SmallShell::getInstance().setSmashName("smash");
  }

  else
    SmallShell::getInstance().setSmashName(args[1]);
}

/////////////////////////////////////// CD COMMAND /////////////////////////////////////
void ChangeDirCommand::execute()
{
  if (this->argsnum > 2)
    cerr << "smash error: cd: too many arguments" << endl;

  // if wanting to go back
  else if (*this->args[1] == '-')
  {
    // if nowhere to go back
    if (*this->last == nullptr)
      cerr << "smash error: cd: OLDPWD not set" << endl;

    // if somewhere to go back
    else
    {
      char *path = getcwd(NULL, 0);

      if (path == nullptr)
      {
        perror("smash error: getcwd failed");
        return;
      }
      else
      {
        if (chdir(*this->last) == -1)
        {
          perror("smash error: chdir failed");
        }

        *this->last = path;
      }
    }
  }

  // normal case
  else
  {
    char *path = getcwd(NULL, 0);
    if (path == nullptr)
    {
      perror("smash error: getcwd failed");
      return;
    }

    if (chdir(args[1]) == -1)
    {
      perror("smash error: chdir failed");
      return;
    }
    *this->last = path;
  }
}
///////////////////////////////////// QUIT COMMAND /////////////////////////////////////
QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), steves(jobs)
{
}

void QuitCommand::execute()
{

  // this->steves->removeFinishedJobs();
  if (this->argsnum > 1 && !strcmp(this->args[1], "kill"))
  {
    cout << "smash: sending SIGKILL signal to " << this->steves->get_jobs_num() << " jobs:" << endl;
    this->steves->killAllJobs();
  }
  exit(0);
}
///////////////////////////////////// JOBS COMMAND /////////////////////////////////////

JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line)
{
  this->m_jobs = jobs;
}

void JobsCommand::execute()
{
  this->m_jobs->printJobsList();
}

///////////////////////////////////// FOREGROUND COMMAND /////////////////////////////////////

ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), m_jobs(jobs) {}

void ForegroundCommand::execute()
{
  // this->m_jobs->removeFinishedJobs();

  // remove specific job
  if (this->argsnum >= 2)
  {
    bool success = false;
    int id = this->get_int_arg(this->args[1], &success, 10);
    if (!success)
    {
      cerr << "smash error: fg: invalid arguments" << endl;
      return;
    }
    JobsList::JobEntry *job = m_jobs->getJobById(id);

    if (job == nullptr)
      cerr << "smash error: fg: job-id " << args[1] << " does not exist" << endl;
    else if (this->argsnum > 2)
    {
      cerr << "smash error: fg: invalid arguments" << endl;
      return;
    }
    else
    {
      pid_t pid = job->get_pid();

      cout << job->get_name() << " " << pid << endl;

      SmallShell::getInstance().curr_pid = pid;

      int res = waitpid(pid, NULL, 0);
      if (res == -1 && errno != ECHILD)
      {
        perror("smash error: waitpid failed");
        return;
      }
      SmallShell::getInstance().curr_pid = 0;
      m_jobs->removeJobById(id);
      return;
    }
  }

  // remove last job
  else if (this->argsnum == 1)
  {
    if (m_jobs->get_jobs_num() == 0)
    {
      cerr << "smash error: fg: jobs list is empty" << endl;
      return;
    }

    else
    {
      char *name = this->m_jobs->getLastJobName();
      pid_t pid = m_jobs->removeLastJob();
      cout << name << " " << pid << endl;
      SmallShell::getInstance().curr_pid = pid;

      int res = waitpid(pid, NULL, 0);
      if (res == -1 && errno != ECHILD)
      {
        perror("smash error: waitpid failed");
        return;
      }
      SmallShell::getInstance().curr_pid = 0;
      return;
    }
  }
}

///////////////////////////////////// KILL COMMAND /////////////////////////////////////
KillCommand::KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), m_jobs(jobs) {}

void KillCommand::execute()
{
  // this->m_jobs->removeFinishedJobs();

  if (argsnum < 3)
  {
    cerr << "smash error: kill: invalid arguments" << endl;
    return;
  }
  else
  {
    bool success1 = false;
    bool success2 = false;
    int sigid = this->get_int_arg(this->args[1] + 1 * sizeof(char), &success1, 10);
    int jobid = this->get_int_arg(this->args[2], &success2, 10);
    JobsList::JobEntry *job;
    if (success2)
    {
      job = this->m_jobs->getJobById(jobid);
      if (job == nullptr)
      {
        cerr << "smash error: kill: job-id " << jobid << " does not exist" << endl;
        return;
      }
    }

    if (!success1 || !success2 || *this->args[1] != '-' || this->argsnum != 3)
    {
      cerr << "smash error: kill: invalid arguments" << endl;
      return;
    }

    pid_t pid = job->get_pid();
    if (kill(pid, sigid) == -1)
    {
      perror("smash error: kill failed");
      return;
    }
    cout << "signal number " << sigid << " was sent to pid " << pid << endl;
  }
}

///////////////////////////////////// CHMOD COMMAND /////////////////////////////////////

ChmodCommand::ChmodCommand(const char *cmd_line) : BuiltInCommand(cmd_line)
{
}

void ChmodCommand::execute()
{
  if (argsnum == 3)
  {
    bool success = false;
    int mode = this->get_int_arg(args[1], &success, 8);
    if (success)
    {
      if (chmod(args[2], mode) == -1)
      {
        perror("smash error: chmod failed");
      }
    }
    else
    {
      cerr << "smash error: chmod: invalid arguments" << endl;
    }
  }
  else
    cerr << "smash error: chmod: invalid arguments" << endl;
}

// ///////////////////////////////////// REDIRECTION COMMAND /////////////////////////////////////

RedirectionCommand::RedirectionCommand(const char *cmd_line) : Command(cmd_line)
{
}

void RedirectionCommand::execute()
{
  const string str(cmd_line);
  size_t pos1 = str.find_first_of('>');
  size_t pos2 = str.find_last_of('>');

  if (pos1 != string::npos && pos1 == pos2) // check later
  {
    char *new_cmd = replace_symb(cmd_line, '>', ' ');
    this->argsnum = _parseCommandLine(new_cmd, args);
    this->cmd_line = new_cmd;
    if (argsnum >= 2)
    {
      SmallShell::getInstance().m_stdout = dup(1);
      if (SmallShell::getInstance().m_stdout == -1)
      {
        perror("smash error: dup failed");
      }
      int fd = open(args[argsnum - 1], O_RDWR | O_CREAT | O_TRUNC, 0777);
      if (fd == -1)
      {
        perror("smash error: open failed");
      }
      if (dup2(fd, 1) == -1)
        perror("smash error: dup2 failed");
      if (close(fd) == -1)
      {
        perror("smash error: close failed");
      }
      SmallShell::getInstance().io_flag = true;
      this->shorten_args();
    }
  }
  else if (pos1 != string::npos && pos1 != pos2)
  { // >> case

    char *new_cmd = replace_symb(cmd_line, '>', ' ');
    this->argsnum = _parseCommandLine(new_cmd, args);
    this->cmd_line = new_cmd;

    if (argsnum >= 2)
    {
      SmallShell::getInstance().m_stdout = dup(1);
      if (SmallShell::getInstance().m_stdout == -1)
      {
        perror("smash error: dup failed");
      }
      int fd = open(args[argsnum - 1], O_RDWR | O_CREAT | O_APPEND, 0777);
      if (fd == -1)
      {
        perror("smash error: open failed");
      }
      if (dup2(fd, 1) == -1)
        perror("smash error: dup2 failed");
      if (close(fd) == -1)
        perror("smash error: close failed");

      SmallShell::getInstance().io_flag = true;
      this->shorten_args();
    }
  }
  char *command_cmd = (char *)malloc((pos1) * sizeof(char));
  for (int i = 0; i < (int)pos1; i++)
  {
    command_cmd[i] = cmd_line[i];
  }

  SmallShell::getInstance().executeCommand(command_cmd);
}

/////////////////////////////////////// PIPE COMMAND /////////////////////////////////////

PipeCommand::PipeCommand(const char *cmd_line, size_t pos) : Command(cmd_line), pos(pos)
{
  int cmd_size = strlen(cmd_line);
  if (cmd_size > (int)this->pos + 1 && cmd_line[(int)pos + 1] == '&')
  {
    this->err_pipe = true;
  }

  // char *cmd1 = (char *)malloc((pos + 1) * sizeof(char));
  // char *cmd2 = (char *)malloc((cmd_size - pos + 1) * sizeof(char));
  // this->parse_pipe(cmd1, cmd2, cmd_size);
  string new_cmd = strdup(cmd_line);
  const char *cmd1 = new_cmd.substr(0, this->pos).c_str();
  const char *cmd2;
  if (this->err_pipe)
  {
    cmd2 = new_cmd.substr(this->pos + 1).c_str();
  }
  else
  {
    cmd2 = new_cmd.substr(this->pos + 2).c_str();
  }
  //cout << "finished parsing: cmd1 - " << cmd1 << "\tcmd2 -" << cmd2 << endl;

  this->command1 = SmallShell::getInstance().CreateCommand(cmd1);
  this->command2 = SmallShell::getInstance().CreateCommand(cmd2);
}

// void PipeCommand::parse_pipe(char *cmd1, char *cmd2, int size)
// {
//   // cout << "parsing pipe cmd..." << endl;

//   for (int i = 0; i < size; i++)
//   {
//     if (i < this->pos)
//     {
//       cmd1[i] = this->cmd_line[i];
//     }
//     else if (i == this->pos + this->err_pipe)
//     {
//       continue;
//     }
//     else if (i > this->pos + this->err_pipe && i < size)
//     {
//       cmd2[i - this->pos - this->err_pipe - 1] = this->cmd_line[i];
//     }
//   }
//   // cout << "finished parsing: cmd1 - " << cmd1 << " cmd2 -" << cmd2 << endl;
// }

void PipeCommand::execute()
{
  int my_pipe[2];
  if (pipe(my_pipe) == -1)
  {
    perror("smash error: pipe failed");
    return;
  }
  // cout << "before children" << endl;

  // first command: writing child
  pid_t pid1 = fork();
  if (pid1 == -1)
  {
    perror("smash error: fork failed");
    return;
  }
  else if (pid1 == 0)
  {

    if (setpgrp() == -1)
    {
      perror("smash error: setpgrp failed");
      exit(0);
    }
    // cout << "\tchild 1 before dup, errpipe val is: " << this->err_pipe << endl;

    if (dup2(my_pipe[1], this->err_pipe + 1) == -1)
    {
      perror("smash error: dup2 failed");
      exit(0);
    }

    if (close(my_pipe[0]) == -1 || close(my_pipe[1]) == -1)
    {
      perror("smash error: close failed");
      exit(0);
    }

    //cout << "\tchild 1 after dup, written in err ? " << this->err_pipe << endl;
    //cerr << "\tchild 1 after dup, written in err ? " << this->err_pipe << endl;

    this->command1->execute();
    exit(0);
  }
  // cout << "before child 2" << endl;

  // second command: reading child
  pid_t pid2 = fork();
  if (pid2 == -1)
  {
    perror("smash error: fork failed");
    return;
  }
  else if (pid2 == 0)
  {

    if (setpgrp() == -1)
    {
      perror("smash error: setpgrp failed");
      exit(0);
    }
    // cout << "\tchild 2 before dup" << endl;

    if (dup2(my_pipe[0], 0) == -1)
    {
      perror("smash error: dup2 failed");
      exit(0);
    }

    if (close(my_pipe[0]) == -1 || close(my_pipe[1]) == -1)
    {
      perror("smash error: close failed");
      exit(0);
    }

    this->command2->execute();
    exit(0);
  }

  // cout << "parent after children: closing pipe" << endl;

  if (close(my_pipe[0]) == -1 || close(my_pipe[1]) == -1)
  {
    perror("smash error: close failed");
    exit(0);
  }

  int res1 = waitpid(pid1, NULL, 0);
  if (res1 == -1 && errno != ECHILD)
  {
    perror("smash error: waitpid failed");
  }

  int res2 = waitpid(pid2, NULL, 0);
  if (res2 == -1 && errno != ECHILD)
  {
    perror("smash error: waitpid failed");
    return;
  }
  // cout << "waited for all" << endl;
}

// void PipeCommand::execute(){
//   int my_pipe[2];
//   if(pipe(my_pipe) == -1){
//     perror("smash error: pipe failed");
//     return;
//   }

//   pid_t pid = fork();

//   if(pid == -1){
//     perror("smash error: fork failed");
//     return;
//   }

//   else if(pid > 0) // im parent - cmd1  - restore in the end pls
//   {
//     cout << "im parent" << endl;

//     if(close(my_pipe[0])== -1) // i will write and not read
//     {
//       perror("smash error: close failed");
//       return;
//     }
//     cout << "im parent: closing pipe" << endl;

//     //save old stdout, stderr
//     SmallShell::getInstance().m_stdout = dup(1);
//     if (SmallShell::getInstance().m_stdout == -1)
//     {
//       perror("smash error: dup failed");
//     }
//     SmallShell::getInstance().m_stderr = dup(2);
//     if (SmallShell::getInstance().m_stderr == -1)
//     {
//       perror("smash error: dup failed");
//     }

//     cout << "im parent: saved old channels" << endl;

//     if(dup2(my_pipe[1], 1 + this->err_pipe) == -1) //cmd1 stdout/stderr to write
//     {
//       perror("smash error: dup2 failed");
//       return;
//     }

//     this->command1->execute();

//     int res = waitpid(pid, NULL, 0);
//     if (res == -1 && errno != ECHILD)
//     {
//       perror("smash error: waitpid failed");
//       return;
//     }
//   }

//   else if(pid == 0) //im baby - cmd2
//   {
// if (execvp(new_args[0], new_args) == -1)
//       {
//         perror("smash error: execvp failed");
//         exit(0);
//       }

//     if(close(my_pipe[1]) == -1) // i will read and not write
//     {
//       perror("smash error: close failed");
//       exit(0);
//     }

//     if(dup2(my_pipe[0], 0) == -1){
//       perror("smash error: dup2 failed");
//       exit(0);
//     }

//     //cmd2 stdin to read
//     this->command2->execute();
//     exit(0);
//   }
// }

// ///////////////////////////////////// TIMEOUT COMMAND /////////////////////////////////////
// TimeoutCommand::TimeoutCommand(const char *cmd_line)
// {
//   for (vector<TimerNode *>::iterator curr = this->wake_queue->begin(); curr 1= this->wake_queue->end(); )
//   {
//     if (time(NULL) > (*curr)->end_time)
//       curr = wake_queue->erase(curr);
//   } else curr++;

//   sort(this->wake_queue->begin(), this->wake_queue->end(), this->cmp);
//   alarm(this->wake_queue->front()->end_time - time(NULL));
// }

// void TimeoutCommand::execute()
// {

//   return;
// }

// TimeoutCommand::TimerNode::TimerNode(time_t duration, pid_t pid) : duration(duration), pid(pid)
// {
//   this->end_time = time(NULL) + duration;
// }

// bool TimeoutCommand::cmp(const TimerNode &p1, const TimerNode &p2)
// {
//   return p1.end_time < p2.end_time;
// }
