#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command
{
  // TODO: Add your data members
protected:
  char **args;
  int argsnum;
  const char *cmd_line;
  const char *old_unstripped;

public:
  Command(const char *cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  // virtual void prepare();
  // virtual void cleanup();
  //  TODO: Add your extra methods if needed
  virtual const char *get_name();
  void shorten_args();
};

class BuiltInCommand : public Command
{
public:
  BuiltInCommand(const char *cmd_line);
  virtual ~BuiltInCommand() {}
  int get_int_arg(char *s, bool *success, int base);
};

class ExternalCommand : public Command
{
public:
  ExternalCommand(const char *cmd_line);
  virtual ~ExternalCommand();
  void execute() override;
  bool is_complex_ext();
  bool is_bg;
};

class PipeCommand : public Command
{
  size_t pos;
  bool err_pipe = false;
  Command *command1, *command2;

public:
  PipeCommand(const char *cmd_line, size_t pos);
  virtual ~PipeCommand() {}
  void execute() override;
  void parse_pipe(char *cmd1, char *cmd2, int size);
};

class RedirectionCommand : public Command
{
  // TODO: Add your data members
public:
  explicit RedirectionCommand(const char *cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  // void prepare() override;
  // void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand
{
private:
  char **last;
  // TODO: Add your data members
public:
  ChangeDirCommand(const char *cmd_line, char **plastPwd) : BuiltInCommand(cmd_line), last(plastPwd) {}
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand
{
public:
  GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

  virtual ~GetCurrDirCommand() {}

  void execute() override;
};

class ShowPidCommand : public BuiltInCommand
{
public:
  ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class RenameSmashCommand : public BuiltInCommand
{
public:
  RenameSmashCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~RenameSmashCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand
{
private:
  JobsList *steves;

public:
  QuitCommand(const char *cmd_line, JobsList *jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

class JobsList
{

public:
  class JobEntry
  {
  private:
    int jobid;
    pid_t pid;
    char *job_name;

  public:
    JobEntry(int jobid, pid_t pid, const char *name);
    int get_id();
    char *get_name();
    pid_t get_pid();
  };

private:
  std::vector<JobEntry *> *m_jobs_list;

  // TODO: Add your data members
public:
  JobsList();
  ~JobsList();
  void addJob(Command *cmd, pid_t pid);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  char *getLastJobName();
  pid_t removeLastJob();
  int get_jobs_num();
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand
{
  // TODO: Add your data members
  JobsList *m_jobs;

public:
  JobsCommand(const char *cmd_line, JobsList *jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand
{
  // TODO: Add your data members
  JobsList *m_jobs;

public:
  KillCommand(const char *cmd_line, JobsList *jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand
{
private:
  JobsList *m_jobs;

public:
  ForegroundCommand(const char *cmd_line, JobsList *jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand
{
public:
  ChmodCommand(const char *cmd_line);
  virtual ~ChmodCommand() {}
  void execute() override;
};

class TimeoutCommand : public BuiltInCommand
{

public:
  class TimerNode
  {
  public:
    time_t end_time, duration;
    pid_t pid;
    TimerNode(time_t duration, pid_t pid);
  };

  TimeoutCommand(const char *cmd_line);
  virtual ~TimeoutCommand() {}
  void execute() override;
  std::vector<TimerNode *> *wake_queue;
  bool cmp(const TimerNode &p1, const TimerNode &p2);
};

class SmallShell
{
private:
  // TODO: Add your data members
  std::string smashName = "smash";
  SmallShell();

public:
  Command *CreateCommand(const char *cmd_line);
  SmallShell(SmallShell const &) = delete;     // disable copy ctor
  void operator=(SmallShell const &) = delete; // disable = operator
  static SmallShell &getInstance()             // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  void setSmashName(std::string name);
  ~SmallShell();
  void executeCommand(const char *cmd_line);

  std::string getName();
  char *lastPwd = nullptr;
  JobsList *m_jobs;
  int m_stdout;
  // int m_stderr;
  bool io_flag;
  // TODO: add extra methods as needed
  void retrieve_io();
  pid_t curr_pid = 0;
};

#endif // SMASH_COMMAND_H_
