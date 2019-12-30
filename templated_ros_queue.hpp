#ifndef TEMPLATED_ROS_QUEUE_HPP
#define TEMPLATED_ROS_QUEUE_HPP
/*
    Author: Abhinav Tripathi <mr.a.tripthi [at] gmail.com> 

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
    WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
    SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
    OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
    OF SUCH DAMAGE.

    Copyright (c) 2019 Abhinav Tripathi.
    All Rights Reserved.
*/

#include <ros/ros.h>
#include <iostream>
#include <mutex>
#include <condition_variable>

#include <boost/bind.hpp>
#include <thread>
using namespace std;
template <typename T> class TempaltedROSQueue //: public boost::enable_shared_from_this<TempaltedROSQueue<T>>
{
private:
    std::string topic_;
    int q_size_;
    std::mutex mutex_;
    ros::Publisher  ros_topic_pub_;
    ros::Subscriber ros_topic_sub_;
    std::thread callback_thread_h_;
    typedef boost::shared_ptr<T const> CSPTR_t;
    CSPTR_t rx_msg_;
    std::condition_variable cv_;
    
    bool pend_on_message_q(uint32_t timeout_ms)
    {
        using namespace std::chrono_literals;
        auto wait_time = std::chrono::steady_clock::now() +  (1ms * timeout_ms);   
        std::unique_lock<std::mutex> lock(mutex_);
        if(timeout_ms)
            // return cv_.wait_until(lock,wait_time,[this] {return true;});
            return cv_.wait_until(lock,wait_time,[this] {return true;});
        else
        {
            cv_.wait(lock);
            return true;
        }
    }
    
    void main_init(int argc, char **argv)
    {
        ros::NodeHandle ros_nh;
        ros::init(argc, argv, APP_LOGGER_NAME);
        
        ros_topic_pub_ = ros_nh.advertise<T>(topic_, q_size_);
        ros_topic_sub_ = ros_nh.subscribe(topic_, 1,&TempaltedROSQueue<T>::callback_fn,this);
        callback_thread_h_ = std::thread(&TempaltedROSQueue<T>::callback_thread,this);
    }

    void callback_fn(const CSPTR_t p_msg_rx)
    {
        rx_msg_ = p_msg_rx;
        cv_.notify_all();
    }

    void callback_thread(void)
    {
        ros::spin();
    }

public:
    ~TempaltedROSQueue()
    {
        
    }
    
    TempaltedROSQueue(std::string topic)
        : topic_(topic)
        , q_size_(100)
    {
        main_init(0,0);
    }
    TempaltedROSQueue(std::string topic,int n)
        : topic_(topic)
        , q_size_(n)
    {
        main_init(0,0);
    }


    TempaltedROSQueue(std::string topic,int argc, char **argv)
        : topic_(topic)
        , q_size_(100)
    {
        main_init(argc,argv);
    }

    TempaltedROSQueue(std::string topic,int n,int argc, char **argv)
        : topic_(topic)
        , q_size_(n)
    {
        main_init(argc,argv);
    }
    bool wait_pop(T& msg, uint32_t timeout_ms)
    {
        bool status = pend_on_message_q(timeout_ms);
        if(status)
            msg = *rx_msg_;
        return status;
    }
    
    // boost::shared_ptr<TempaltedROSQueue<T>> get_shared_ptr(void)
    // {
    //     return this->shared_from_this();
    // }

};
#endif /* #ifndef TEMPLATED_ROS_QUEUE_HPP */