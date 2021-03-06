//
//  main.cpp
//  qqlogin
//
//  Created by Scott Deng on 9/12/15.
//  Copyright (c) 2015 github. All rights reserved.
//

#include <iostream>
#include "qqlogin.h"
#include "fetcher.hpp"
#include "threadtool.h"
#include <utility>
#include <vector>
void thread_main(threadtool::Threadsafe_queue<std::string>* qq_queue, std::string* qq, std::string* skey);
int main(int argc, const char * argv[]) {
  // insert code here...
  curl_global_init(CURL_GLOBAL_ALL);
  mongo::client::GlobalInstance mongoguard;
  
  threadtool::Threadsafe_queue<std::string>* qq_job_list = new threadtool::Threadsafe_queue<std::string>;
  std::string seed_qq;
  std::string session_qq;
  std::string session_skey;
  int thread_count;
  
  std::cout<<"Please enter the initial qq number as a crawling seed"<<std::endl;
  std::cin>>seed_qq;
  if(seed_qq.empty()) return -1;
  std::cout<<"Please enter a logined qq number"<<std::endl;
  std::cin>>session_qq;
  if(session_qq.empty()) return -1;
  std::cout<<"Please enter corresponend skey"<<std::endl;
  std::cin>>session_skey;
  if(session_skey.empty()) return -1;
  std::cout<<"Please enter number of thread"<<std::endl;
  std::cin>>thread_count;
  if(thread_count < 1) return -1;
  
  qq_job_list->push(seed_qq);
  if(thread_count > 1){
    
    for(int i = 0; i < (thread_count - 1); i++){
      std::thread new_thread(thread_main,qq_job_list, &session_qq, &session_skey);
      new_thread.detach();
    }
  }
  
  thread_main(qq_job_list, &session_qq, &session_skey);
  
error_cleanup:
  curl_global_cleanup();
  return EXIT_SUCCESS;
}

void thread_main(threadtool::Threadsafe_queue<std::string>* qq_queue, std::string* qq, std::string*skey){
  
  mongo::DBClientConnection client;

  client.connect("localhost");
  printf("successfully connected to the database\n");
  auto qq_num = *qq;
  auto qq_skey = *skey;
  qqlogin::QQ_info new_qq(qq_num, qq_skey);
  fetch::Fetcher new_fetcher(new_qq, qq_queue);
  while(1){
    auto it = new_fetcher.toFiltered_json(*(qq_queue->wait_pop()))["msglist"][0];
    if (it.empty() == 0){
      fetch::Shuoshuo new_shuoshuo(it);
      auto it2 = new_shuoshuo.toBSON();
      client.insert("dbs.qq", it2);
    }
  }
}
