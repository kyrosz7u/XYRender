//
// Created by kyros on 2023/6/4.
//

#ifndef XEXAMPLE_SINGLETON_DECORATOR_H
#define XEXAMPLE_SINGLETON_DECORATOR_H

// 更好的单例模式实现：需要单例化的类用这个装饰器包装下即可


template<class T>
class SingletonDecorator
{
public:
    T& Instance()
    {
        static T instance;
        return instance;
    }
    SingletonDecorator(const SingletonDecorator&) = delete;
    SingletonDecorator& operator=(const SingletonDecorator&) = delete;
    SingletonDecorator(SingletonDecorator&&) = delete;
protected:
    SingletonDecorator() {}
};

#endif //XEXAMPLE_SINGLETON_DECORATOR_H
