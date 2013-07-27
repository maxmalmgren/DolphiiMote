#ifndef DOLPHIIMOTE_GLUE_H
#define DOLPHIIMOTE_GLUE_H

#include <functional>
#include <vector>
#include <mutex>
#include <stdint.h>

namespace WiimoteReal {

template <typename T>
  class listener_collection
  {
    public:
      template <typename TFunc, typename TK>
      void notify(TFunc func, TK arg1)
      {
        std::function<void(T*, TK)> resolved_func = std::mem_fn(func);
        std::lock_guard<std::recursive_mutex> lock(callback_lock);

        for(T& listener : listeners)
          resolved_func(&listener, arg1);
      }

      template <typename TFunc, typename TK, typename TX>
      void notify(TFunc func, TK arg1, TX arg2)
      {
        std::function<void(T*, TK, TX)> resolved_func = std::mem_fn(func);
        std::lock_guard<std::recursive_mutex> lock(callback_lock);

        for(T& listener : listeners)
          resolved_func(&listener, arg1, arg2);
      }

      template <typename TFunc, typename TK, typename TX, typename TY, typename TZ>
      void notify(TFunc func, TK arg1, TX arg2, TY arg3, TZ arg4)
      {
        std::function<void(T*, TK, TX, TY, TZ)> resolved_func = std::mem_fn(func);
        std::lock_guard<std::recursive_mutex> lock(callback_lock);

        for(T& listener : listeners)
          resolved_func(&listener, arg1, arg2, arg3, arg4);
      }

      void add(T& listener)
      {
        std::lock_guard<std::recursive_mutex> lock(callback_lock);
        listeners.push_back(listener);
      }

      private:
        std::vector<T> listeners;
        std::recursive_mutex callback_lock;
  };

  template <class T>
  class listener_collection<std::weak_ptr<T>>
  {
  public:
    template <typename TFunc, typename TK>
    void notify(TFunc func, TK arg1)
    {
      std::function<void(T*, TK)> resolved_func = std::mem_fn(func);

	  remove_expired_listeners();

	  for(std::weak_ptr<T> &listener : listeners)
	  {
		  auto& locked = listener.lock();
		  resolved_func(locked.get(), arg1);
	  }
	}

    template <typename TFunc, typename TK, typename TX>
    void notify(TFunc func, TK arg1, TX arg2)
    {
      std::function<void(T*, TK, TX)> resolved_func = std::mem_fn(func);

	    remove_expired_listeners();

      for(std::weak_ptr<T> &listener : listeners)
	    {
		    auto& locked = listener.lock();
		    resolved_func(locked.get(), arg1, arg2);
	    }
    }

    template <typename TFunc, typename TK, typename TX, typename TY, typename TZ>
    void notify(TFunc func, TK arg1, TX arg2, TY arg3, TZ arg4)
    {
      std::function<void(T*, TK, TX, TY, TZ)> resolved_func = std::mem_fn(func);

	    remove_expired_listeners();

      for(std::weak_ptr<T> &listener : listeners)
	    {
        auto& locked = listener.lock();
        resolved_func(locked.get(), arg1, arg2, arg3, arg4);
      }
    }

	void remove_expired_listeners()
	{
		listeners.erase(std::remove_if(listeners.begin(), listeners.end(), [](std::weak_ptr<T> &ptr) -> bool { return ptr.expired(); }), listeners.end());
	}

    void add(std::weak_ptr<T> listener)
    {
      listeners.push_back(listener);
    }

  private:
    std::vector<std::weak_ptr<T>> listeners;
  };

  class wiimote_listener
  {
  public:
    virtual void data_received(int wiimote_number, const u16 channel, const void* const data, const u32 size) = 0;
    virtual void wiimote_connection_changed(int wiimote_number, bool connected) = 0;
    virtual ~wiimote_listener() { }
  };
}

#endif DOLPHIIMOTE_GLUE_H