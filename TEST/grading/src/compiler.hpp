#pragma once
#include "util.hpp"
#include "exception.hpp"
#include <unistd.h>
#include <memory>
#include <thread>
#include <chrono>
#include <future>
#include <condition_variable>
#include <fcntl.h>
#include <sys/wait.h>

namespace Grading{
    class Executer{
        protected:
            std::string target_path_;           //실행 대상 경로
            bool done_;                         //프로그램이 잘 실행되었을 경우 True
            bool time_out_;                     //프로그램이 시간초과가 났을 경우 True

        public:
            Executer(const std::string& target_path)
                     :target_path_(target_path){
                        done_ = false;
                        time_out_ = false;
                     };

            ~Executer(){
                execute("rm %s", target_path_.c_str());
            }

            template<typename T>
            int run(const std::string& data_path, const std::string& result_path, const std::string& log_path, T time_limit){
                /*
                *   target_path를 실행하고 그 결과를 result_path와 log_path에 기록한다.
                *   args:
                *       data_path:   실행을 위한 데이터 경로
                *       result_path: 실행 결과가 기록될 파일의 경로
                *       log_path:    실행 도중 생기는 warning이나 error를 기록할 로그의 경로
                *       time_limit:  시간 제한
                *   return:
                *       정상 실행이 종료된 경우 0을 반환한다.
                *   
                *   except:
                *       RuntimeExecption: 실행 도중 오류가 난 경우 반환
                */

                done_ = false;
                time_out_ = false;

                int pid = fork();
                int input_file_descriptor = open(data_path.c_str(), O_RDONLY);
                int output_file_descriptor = open(result_path.c_str(), O_CREAT | O_RDWR, 0666);
                int log_file_descriptor = open(log_path.c_str(), O_WRONLY);

                if(input_file_descriptor < 0) throw CantOpenFileException(data_path);
                if(output_file_descriptor < 0) throw CantOpenFileException(result_path);
                if(log_file_descriptor < 0) throw CantOpenFileException(log_path);

                if(pid < 0){
                    //fork가 정상적으로 시행이 안되었을 때
                }
                else if(pid == 0){
                    //자식 프로세스 내부
                    
                    //입력 파이프라인
                    dup2(input_file_descriptor, STDIN_FILENO);
                    dup2(output_file_descriptor, STDOUT_FILENO); //STDOUT이 잘 해결되지 않고 있음
                    dup2(log_file_descriptor, STDERR_FILENO);

                    //파일 실행
                    execl(target_path_.c_str(), target_path_.c_str(), NULL);
                }
                else{
                    //부모 프로세스 내부
                    
                    //자식 프로세스 대기
                    int statloc = 0;
                    auto wait_child = std::async(std::launch::async, 
                    [&](int fpid, int* fstatloc, int foption){
                        waitpid(fpid, fstatloc, foption);
                        done_ = true;
                        return;
                    }, 
                    pid, &statloc, 0/*option*/);
                    

                    std::future_status status = wait_child.wait_for(time_limit);

                    if(status == std::future_status::timeout){
                        close(input_file_descriptor);
                        close(output_file_descriptor);
                        close(log_file_descriptor);
                        execute("kill -9 %d", pid);
                        return 1;
                    }

                    return 0;

                }
                return -1;
            }
    };

    class Compiler{
        protected:
            std::string format_;                 //컴파일 명령어
            std::string option_;                 //컴파일러 옵션

            std::string target_path_;            //컴파일 대상 경로
            std::string result_path_;            //컴파일 결과물 경로
            std::string log_path_;               //컴파일 로그 파일 경로

        public:
            Compiler(const std::string& format,
                     const std::string& option, 
                     const std::string& target_path,
                     const std::string& result_path,
                     const std::string& log_path)
                     : format_(format),
                       option_(option),
                       target_path_(target_path),
                       result_path_(result_path),
                       log_path_(log_path){}

            virtual std::unique_ptr<Executer> compile() const = 0;
            virtual std::string get_extension() const = 0;
            virtual ~Compiler(){};
    };

    class CppCompiler: public Compiler{
        public:
            CppCompiler(const std::string& option, 
                        const std::string& target_path,
                        const std::string& result_path,
                        const std::string& log_path)
                        : Compiler("g++ %s -o %s %s 2> %s",
                            option, target_path, result_path, log_path){};

            virtual std::unique_ptr<Executer> compile() const override{
                /*
                *   compile을 실행하고 무사히 실행한 경우 Executer의 unique_ptr을 반환한다.
                *   
                *   return:
                *       unique_ptr<Executer> 실행할 수 있는 파일의 경로를 가지고 있는 Executer 포인터
                *
                *   except:
                *       NoFileException: 컴파일 할 수 있는 파일을 확인하지 못했을 경우 
                *       CompileException: 컴파일 도중 out파일이 제대로 생성되지 못한 경우 반환
                */

                // 파일이 존재하지 않는 경우 예외 발생
                if(access(target_path_.c_str(), F_OK)) throw NoFileException();

                int result = execute(format_, 
                target_path_.c_str(), result_path_.c_str(), option_.c_str(), log_path_.c_str());

                // 정상적으로 컴파일 되지 않은 경우
                if(result != 0) throw CompileException();

                return std::make_unique<Executer>(result_path_);
            }

            virtual std::string get_extension() const override {
                return "cpp";
            }
            
            virtual ~CppCompiler(){};
    };
}