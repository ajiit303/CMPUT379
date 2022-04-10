// Created by Ajiitesh Gupta on 2022/04/08
 
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>
#include <regex>
#include <thread>
#include <mutex>
#include <chrono>
#include <map>
#define NRES_TYPES 10 // max resources typ
#define NJOBS 25 // max jobs
#define DEBUG false

using namespace std;

// defining helper code for string splitting

vector<string> split(string s, char delimiter) {
  istringstream ss(s);
  vector<string> tokens;
  string buffer;

  while (getline(ss, buffer, delimiter)) {
    tokens.push_back(buffer);
  }

  return tokens;
}

//defining states for jobs

typedef enum {
  STATE_WAIT,
  STATE_RUN,
  STATE_IDLE
} JobState;

// defining states for resources

typedef enum {
  STATE_FREE,
  STATE_BUSY
} ResourceState;

// defining structures for resource

struct Resource {
  std::string name;
  int units; // how many instances are avaliable
  int held; // how many instance are currently being held
  ResourceState state;
};

// defining structures for jobs

struct Job {
  std::string name;
  int busy_time;
  int idle_time;
  std::thread::id jid;
  std::vector<Resource> required_resources;
  JobState state;
  int time_wait; // amout of time spent on waiting for resources
  int cnt; // number of iteration
};

// Methods to implement Assignment 4

void start_simulator(std::string, int, int);
void job_runner(std::string, int);
void monitor_runner(int);
void print_running_job(std::string, std::thread::id, int, int);
void print_resources_info();
void print_jobs_info();
void debug_before_release_resource(std::string, std::string);
void debug_after_release_resource(std::string, std::string);
void debug_before_acquire_resource(std::string, std::string);
void debug_after_acquire_resource(std::string, std::string);
void debug_fail_acquire_resouce(std::string, std::string);
std::tuple<std::map<std::string, Resource>, std::map<std::string, Job>> parse_input_file(std::string);
map<string, Resource> resources_map;
map<string, Job> jobs_map;
vector<thread> workers;
bool worker_terminal;
chrono::time_point<chrono::high_resolution_clock> start_time;

mutex resource_mutex;
mutex job_mutex;

map<JobState, string> state_name_map = {
  { STATE_IDLE, "IDLE" },
  { STATE_WAIT, "WAIT" },
  { STATE_RUN, "RUN" }
};

int main(int argc, char *argv[]) {
  start_time = chrono::high_resolution_clock::now();

  if (argc < 4) {
    cout << "Invalid argument" << endl;
    cout << "./a4w22 inputFile monitorTime NITER" << endl;
  }

  string input_file = argv[1];
  int monitoring_interval = stoi(argv[2]);
  int num_iteration = stoi(argv[3]);

  start_simulator(input_file, num_iteration, monitoring_interval);

  return 0;
}

void start_simulator(string input_file, int num_iteration, int monitoring_interval) {
  tie(resources_map, jobs_map) = parse_input_file(input_file);

  worker_terminal = false; // indicator for monitor thread to exit

  // create workers thread
  for (auto ele : jobs_map) {
    workers.push_back(thread(job_runner, ele.first, num_iteration));
  }

  // create monitor thread
  thread monitor_thread(monitor_runner, monitoring_interval);

  for (auto& worker : workers) {
    worker.join();
  }

  // terminate monitor thread
  worker_terminal = true;
  monitor_thread.join();

  // wrap up and terminate
  print_resources_info();
  print_jobs_info();

  auto current_time = chrono::high_resolution_clock::now();
  cout << "Running time= " << chrono::duration_cast<chrono::milliseconds>(current_time - start_time).count() << " msec" << endl;
}

void job_runner(string job_name, int num_iteration) {
  Job current_job = jobs_map[job_name];
  jobs_map[job_name].jid = this_thread::get_id();
  jobs_map[job_name].cnt = 0;
  current_job = jobs_map[job_name];

  for (int run = 0; run < num_iteration; run++) {
    auto start_acquire_resources = chrono::high_resolution_clock::now();

    // when job does not require resource, just skip
    if (current_job.required_resources.empty()) {
      print_running_job(current_job.name, current_job.jid, run+1, 0);
      jobs_map[job_name].cnt += 1;
      continue;
    }

    // update state to WAIT
    job_mutex.lock();
    jobs_map[job_name].state = STATE_WAIT;
    job_mutex.unlock();

    while (true) {
      if (resource_mutex.try_lock()) {
        bool flag = false; // true -> lack of resource, false -> all good and acquire resources
        for (auto r : current_job.required_resources) {
          if (!((resources_map[r.name].units - resources_map[r.name].held) >= r.units)) {
            debug_fail_acquire_resouce(current_job.name, r.name);
            flag = true;
            break;
          }
        }
        if (flag) {
          resource_mutex.unlock();
          this_thread::sleep_for(chrono::milliseconds(100));
        } else {
          for (auto r : current_job.required_resources) {
            debug_before_acquire_resource(current_job.name, r.name);
            resources_map[r.name].held += r.units; // all resoucres are avalibale, hold resouce.
            debug_after_acquire_resource(current_job.name, r.name);
          }
          resource_mutex.unlock();
          break;
        }
      } else {
        this_thread::sleep_for(chrono::milliseconds(100));
      }
    }
    auto finish_acquire_resources = chrono::high_resolution_clock::now();
    jobs_map[job_name].time_wait += chrono::duration_cast<chrono::milliseconds>(finish_acquire_resources - start_acquire_resources).count();

    // update state to RUN
    job_mutex.lock();
    jobs_map[job_name].state = STATE_RUN;
    job_mutex.unlock();

    // simulate busy time
    this_thread::sleep_for(chrono::milliseconds(current_job.busy_time));

    // release resource
    for (auto r : current_job.required_resources) {
      lock_guard<mutex> lock(resource_mutex);
      debug_before_release_resource(current_job.name, r.name);
      resources_map[r.name].held -= r.units;
      debug_after_release_resource(current_job.name, r.name);
    }

    // update state to IDLE
    job_mutex.lock();
    jobs_map[job_name].state = STATE_IDLE;
    job_mutex.unlock();

    // simulate idle time
    this_thread::sleep_for(chrono::milliseconds(current_job.idle_time));

    // wrap up current iteration
    auto current_time = chrono::high_resolution_clock::now();
    auto time_spent = chrono::duration_cast<chrono::milliseconds>(current_time - start_time).count();
    print_running_job(current_job.name, current_job.jid, run+1, time_spent);
    jobs_map[job_name].cnt += 1;
  }
}

void monitor_runner(int interval) {
  while (!worker_terminal) {
    // TODO lock_guard is casuing weird behavior on gcc 5.4
    // lock_guard<mutex> lock(job_mutex); // make sure no job state change when printing
    job_mutex.lock();
    stringstream wait_buf;
    stringstream run_buf;
    stringstream idle_buf;
    wait_buf << "    [WAIT] ";
    run_buf << "    [RUN] ";
    idle_buf << "    [IDLE] ";
    for (auto ele : jobs_map ) {
      Job j = ele.second;
      switch (j.state) {
        case STATE_WAIT:
          wait_buf << j.name << " ";
          break;
        case STATE_RUN:
          run_buf << j.name << " ";
          break;
        case STATE_IDLE:
          idle_buf << j.name << " ";
          break;
      }
    }
    job_mutex.unlock();
    cout << "monitor:" << "\n" << wait_buf.str() << "\n" << run_buf.str() << "\n" << idle_buf.str() << endl;
    this_thread::sleep_for(chrono::milliseconds(interval));
  }
}

void print_running_job(string name, thread::id jid, int iteration, int time) {
  cout << "job: " << name << " (jid= " << jid << ", iter= " << iteration << ", time= " << time << " msec)" << endl;
}

void print_resources_info() {
  stringstream ss;
  ss << "System Resources:\n";
  for (auto ele : resources_map) {
    Resource r = ele.second;
    ss << "    " << ele.first << ": (maxAvail= " << r.units << " , held= " << r.held << ")" << endl;
  }
  cout << ss.str();
}

void print_jobs_info() {
  stringstream ss;
  ss << "System Jobs:" << endl;
  int cnt = 0;

  for (auto ele : jobs_map) {
    Job j = ele.second;
    ss << "[" << cnt << "] " << j.name << endl;
    ss << "    (" << state_name_map[j.state] << " runTime: " << j.busy_time << " msec, idleTime= " << j.idle_time << " msec):" << endl;
    ss << "    (jid= " << j.jid << ")" << endl;
    for (auto r : j.required_resources) {
      ss << "    " << r.name << ": (needed= " << r.units << ", held= " << r.held << ")" << endl;
    }
    ss << "    (RUN: " << j.cnt << " times, WAIT: " << j.time_wait << " msec)" << endl;
    cnt += 1;
  }

  cout << ss.str();
}

// parse input file
tuple<map<string, Resource>, map<string, Job>> parse_input_file(string input_file) {
  vector<string> lines;
  ifstream file(input_file);
  string buffer;

  // seperate line by space
  while (getline(file, buffer)) {
    lines.push_back(buffer);
  }

  map<string, Resource> r_map;
  map<string, Job> j_map;
  for (auto line : lines) {
    // skip comment or empty line
    if (line.find("#") == string::npos && !line.empty()) {
      regex ws_re("\\s+");
      vector<string> placeholder {
        sregex_token_iterator(line.begin(), line.end(), ws_re, -1), {}
      };

      if (placeholder[0] == "resources") {
        for (int i = 1; i < (long long int)placeholder.size(); i++) {
          auto tokens = split(placeholder[i], ':');
          int units = stoi(tokens[1]);
          r_map.insert({ tokens[0], { tokens[0], units, 0, STATE_FREE } });
        }
      } else if (placeholder[0] == "job") {
        Job job = { placeholder[1], stoi(placeholder[2]), stoi(placeholder[3]) };
        vector<Resource> job_required_resource;
        if (placeholder.size() > 4) {
          // only parse requires resources for job when there is any
          for (int i = 4; i < (long long int)placeholder.size(); i++) {
            auto tokens = split(placeholder[i], ':');
            job.required_resources.push_back({ tokens[0], stoi(tokens[1]), 0 }); // TODO
          }
        }
        job.state = STATE_IDLE; // STATE_WAIT;
        j_map.insert({ job.name, job });
      }
    }
  }

  return make_tuple(r_map, j_map);
}

void debug_before_release_resource(string job, string resource) {
  if (DEBUG) {
    cout << job << " released " << resource << endl;
    cout << resource << ": maxAvaliable: " << resources_map[resource].units << " held: " << resources_map[resource].held << endl;
  }
}

void debug_after_release_resource(string job, string resource) {
  if (DEBUG) {
    cout << job << " released " << resource << endl;
    cout << resource << ": maxAvaliable: " << resources_map[resource].units << " held: " << resources_map[resource].held << endl;
    cout << endl;
  }
}

void debug_before_acquire_resource(string job, string resource) {
  if (DEBUG) {
    cout << job << " before acquiring " << resource << endl;
    cout << resource << ": maxAvaliable: " << resources_map[resource].units << " held: " << resources_map[resource].held << endl;
  }
}

void debug_after_acquire_resource(string job, string resource) {
  if (DEBUG) {
    cout << job << " after acquiring " << resource << endl;
    cout << resource << ": maxAvaliable: " << resources_map[resource].units << " held: " << resources_map[resource].held << endl;
    cout << endl;
  }
}

void debug_fail_acquire_resouce(string job, string resource) {
  if (DEBUG) {
    cout << job << " fail to acquire " << resource << endl;
    cout << resource << ": maxAvaliable: " << resources_map[resource].units << " held: " << resources_map[resource].held << endl;
    cout << endl;
  }
}
