#include "test_common.h"

#include <thread>

#include "Common/BBox.hpp"
#include "Common/DynamicArray.hpp"
#include "Common/HashMap.hpp"
#include "Common/HashTable.hpp"
#include "Common/HashSet.hpp"
#include "Common/List.hpp"
#include "Common/Log.hpp"
#include "Common/Matrix.hpp"
#include "Common/MemAllocator.hpp"
#include "Common/Object.hpp"
#include "Common/Protocol.hpp"
#include "Common/Publisher.hpp"
#include "Common/SharedPtr.hpp"
#include "Common/Singleton.hpp"
#include "Common/String.hpp"
#include "Common/Subscriber.hpp"

TEST_F(TestCommon, BBoxTest)
{
    using EgLab::Common::BBox;
    using EgLab::Common::Vector2d;
    using EgLab::Common::Vector3d;

    BBox<double, 3> box;
    EXPECT_TRUE(box.isEmpty());
    EXPECT_FALSE(box.contains(Vector3d(0.0, 0.0, 0.0)));

    box.addPoint(Vector3d(2.0, -1.0, 4.0));
    EXPECT_FALSE(box.isEmpty());
    EXPECT_TRUE(box.contains(Vector3d(2.0, -1.0, 4.0)));
    EXPECT_FALSE(box.contains(Vector3d(2.1, -1.0, 4.0)));

    box.addPoint(Vector3d(2.0, -1.0, 4.0));
    box.addPoint(Vector3d(-3.0, 5.0, 1.0));
    box.addPoint(Vector3d(7.0, -4.0, 9.0));

    EXPECT_EQ(box.min()[0], -3.0);
    EXPECT_EQ(box.min()[1], -4.0);
    EXPECT_EQ(box.min()[2], 1.0);
    EXPECT_EQ(box.max()[0], 7.0);
    EXPECT_EQ(box.max()[1], 5.0);
    EXPECT_EQ(box.max()[2], 9.0);
    EXPECT_TRUE(box.contains(Vector3d(0.0, 0.0, 5.0)));
    EXPECT_FALSE(box.contains(Vector3d(8.0, 0.0, 5.0)));

    BBox<double, 2> box2;
    box2.addPoint(Vector2d(-2.0, 3.0));
    box2.addPoint(Vector2d(4.0, -1.0));
    EXPECT_TRUE(box2.contains(Vector2d(-2.0, -1.0)));
    EXPECT_FALSE(box2.contains(Vector2d(-2.1, 0.0)));

    box.reset();
    EXPECT_TRUE(box.isEmpty());
    EXPECT_FALSE(box.contains(Vector3d(0.0, 0.0, 0.0)));
}

TEST_F(TestCommon, testSingle)
{
    class ClassA : public EgLab::Common::Singleton<ClassA>
    {
    public:
        void setA(int _a)
        {
            a = _a;
        }
        int getA()
        {
            return a;
        }

    private:
        int a;
    };

    ClassA& a1 = ClassA::instance();
    a1.setA(2);
    EXPECT_EQ(a1.getA(), 2);

    ClassA& a2 = ClassA::instance();
    a2.setA(4);
    EXPECT_EQ(a1.getA(), 4);
}

int testWithOneParam(int a)
{
    return 1;
}

void testWithTwoParam(int a, float b)
{
    // return 1;
}

int testWithZeroParam()
{
    return 1;
}

TEST_F(TestCommon, Log)
{
    LOG(INFO) << "info";
    LOG(WARNING) << "test";
    LOG(ERROR) << "test";
    LOG(FATAL) << "test";

    int a = 4;
    LOG_CALL(testWithOneParam, a);
    LOG_CALL(testWithTwoParam, a, a);
    LOG_CALL(testWithZeroParam);
}

TEST_F(TestCommon, String)
{
    EgLab::Common::String str;
    str.append("test");
    EXPECT_EQ(str.c_str()[0], "test"[0]);
    str.append("abcdefghijklmn");
    EXPECT_EQ(str.c_str()[0], "test"[0]);
    std::string cppstr = "testabcdefghijklmn";
    for (int i = 0; i < cppstr.size(); i++)
    {
        EXPECT_EQ(cppstr.c_str()[i], str.c_str()[i]);
    }

    EgLab::Common::String a("a");
    EgLab::Common::String b("b");
    auto c = a + b;
    EXPECT_EQ(c.c_str()[0], 'a');
    EXPECT_EQ(c.c_str()[1], 'b');
}

#include <any>
#include <chrono>
#include <vector>

TEST_F(TestCommon, anyTest)
{
    return; // just for test
    using namespace std;
    using namespace std::chrono;
    const int N = 10000000; // 测试次数

    // 准�?�数�?
    std::vector<int> data(N);
    for (int i = 0; i < N; ++i)
    {
        data[i] = i;
    }

    // 测试 std::any �?�?
    vector<any> anys(N);
    for (int i = 0; i < N; ++i)
    {
        anys[i] = data[i];
    }

    volatile int sum_any = 0; // 防�??优化
    auto start_any = high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
    {
        sum_any += any_cast<int>(anys[i]);
    }
    auto end_any = high_resolution_clock::now();
    auto duration_any = duration_cast<milliseconds>(end_any - start_any).count();

    // 测试 void* �?�?
    vector<void*> void_ptrs(N);
    for (int i = 0; i < N; ++i)
    {
        void_ptrs[i] = &data[i];
    }

    volatile int sum_void = 0; // 防�??优化
    auto start_void = high_resolution_clock::now();
    for (int i = 0; i < N; ++i)
    {
        sum_void += *(static_cast<int*>(void_ptrs[i]));
    }
    auto end_void = high_resolution_clock::now();
    auto duration_void = duration_cast<milliseconds>(end_void - start_void).count();

    cout << "std::any cost: " << duration_any << " ms, sum = " << sum_any << endl;
    cout << "void* cost: " << duration_void << " ms, sum = " << sum_void << endl;
}

TEST_F(TestCommon, MemAllocateTest)
{
    // return; // the case cannot pass beacuse of the complier changed
    EgLab::Common::StaticSizeAllocator<int> allocator;
    int* b = static_cast<int*>(allocator.alloc());
    size_t testNum = 100000;
    std::vector<int*> p(testNum);
    for (int i = 0; i < testNum; i++)
    {
        int* a = static_cast<int*>(allocator.alloc());
        p[i] = a;
    }

    for (int i = 0; i < testNum; i++)
    {
        allocator.free(p[i]);
    }
    allocator.free(b);
}

TEST_F(TestCommon, MatrixTest)
{
    EgLab::Common::Matrix<double, 4, 4> m4(3, 4, 1, 2, 3, 4, 5, 4, 7, 1, 5, 4, 1, 6, 5.25, 76);
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            LOG(INFO) << m4[i][j];
        }
    }
}

TEST_F(TestCommon, ListTest)
{
    EgLab::Common::List<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);
    list.pushBack(4);

    list.popBack();
    list.popBack();

    EgLab::Common::List<int>::Node* current = list.getHead();
    while (current != nullptr)
    {
        LOG(INFO) << current->data;
        current = current->next;
    }
}

TEST_F(TestCommon, ListItTest)
{
    EgLab::Common::List<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);
    list.pushBack(4);

    auto it = list.begin();
    int i = 1;
    while (it.hasNext())
    {
        int& value = it.next();
        EXPECT_EQ(value, i);
        i++;
    }

    i = 1;

    for (auto it = list.begin(); it.hasNext(); it.next())
    {
        int& value = it.data();
        EXPECT_EQ(value, i);
        i++;
    }
}

TEST_F(TestCommon, HashSet)
{
    EgLab::Common::HashSet<int> HashSet;
    int data[] = {1,       3,    2,    1,   4,     5,       6,     6,   7,     1,     12,
                  2,       34,   5314, 5,   13246, 1346,    1346,  134, 6134,  513,   45123,
                  423,     412,  312,  412, 534,   61,      326,   134, 51345, 12233, 2431,
                  3,       12,   312,  41,  25,    134,     5,     413, 5613,  56,    132,
                  6,       1346, 1,    235, 123,   5,       12356, 143, 6,     1342,  76,
                  2548769, 758,  9,    79,  5768,  3657,    6245,  6,   423,   5321,  4,
                  21,      312,  4,    125, 34,    7626243, 5,     13};

    for (int i = 0; i < 73; i++)
    {
        HashSet.insert(data[i]);
    }

    int i = 0;
    for (auto it = HashSet.begin(); it != HashSet.end(); ++it)
    {
        LOG(INFO) << i << " " << *it;
        i++;
    }

    // EXPECT_TRUE(HashSet.insert(1));
    // EXPECT_TRUE(HashSet.insert(2));
    // EXPECT_FALSE(HashSet.insert(1)); // 重复插入
    // EXPECT_EQ(HashSet.size(), 2);
}

TEST_F(TestCommon, HashMap)
{
    EgLab::Common::HashMap<int, int> map;
    map.insert({1, 1});
    map.insert({1, 2});
    map.insert({1, 4});
    map.insert({2, 1});
    for (auto it : map)
    {
        LOG(INFO) << it.first << " " << it.second;
    }
    map.erase(2);
    map.erase(3);
    for (auto it : map)
    {
        LOG(INFO) << it.first << " " << it.second;
    }
}

TEST_F(TestCommon, HashMap_operator)
{
    EgLab::Common::HashMap<int, EgLab::Common::String> map;
    map[0] = "aaa";
    EXPECT_EQ(map[0], "aaa");
    map[0] = "bbb";
    EXPECT_EQ(map[0], "bbb");
}

TEST_F(TestCommon, mix)
{
    int data[] = {1,       3,    2,    1,   4,     5,       6,     6,   7,     1,     12,
                  2,       34,   5314, 5,   13246, 1346,    1346,  134, 6134,  513,   45123,
                  423,     412,  312,  412, 534,   61,      326,   134, 51345, 12233, 2431,
                  3,       12,   312,  41,  25,    134,     5,     413, 5613,  56,    132,
                  6,       1346, 1,    235, 123,   5,       12356, 143, 6,     1342,  76,
                  2548769, 758,  9,    79,  5768,  3657,    6245,  6,   423,   5321,  4,
                  21,      312,  4,    125, 34,    7626243, 5,     13};

    EgLab::Common::DynamicArray<EgLab::Common::List<int>> test;
    for (int i = 0; i < 73; i++)
    {
        EgLab::Common::List<int> l;
        test.resize(i + 1);
        l.pushBack(data[i]);
        test.pushBack(move(l));
    }
}

TEST_F(TestCommon, logMatrix)
{
    // clang-format off
    EgLab::Common::Matrix4f m(
        1,0,0,351236501,
        0,565125125,0,0,
        0,0,1.12412515,0,
        0,0,0,1.432141
    );
    // clang-format on

    LOG(INFO) << m;
}

TEST_F(TestCommon, sharedPtr)
{
    class A
    {
    public:
        A(int v) : _v(v)
        {
            LOG(INFO) << "A()" << " : " << _v;
        }

        A()
        {
            LOG(INFO) << "A()" << " : ";
        }

        ~A()
        {
            LOG(INFO) << "~A()" << " : " << _v;
        }

    private:
        int _v;
    };

    EgLab::Common::SharedPtr<A> a(1);
    EgLab::Common::SharedPtr<A> e = EgLab::Common::makeShared<A>(2);
    EgLab::Common::SharedPtr<A> b(a);
    EgLab::Common::SharedPtr<A> c(2);
    EgLab::Common::SharedPtr<A> d(3);
    a = nullptr;
    b = nullptr;
    c = d;
    c = b;

    LOG(INFO) << "end";
}

TEST_F(TestCommon, uniquePtr)
{
    class A
    {
    public:
        A(int v) : _v(v)
        {
            LOG(INFO) << "A()" << " : " << _v;
        }

        A()
        {
            LOG(INFO) << "A()" << " : ";
        }

        ~A()
        {
            LOG(INFO) << "~A()" << " : " << _v;
        }

    private:
        int _v;
    };
    EgLab::Common::UniquePtr<A> cc;
    cc = EgLab::Common::makeUnique<A>(5);
}

TEST_F(TestCommon, Matrix_Identity)
{
    EgLab::Common::Matrix4f mat = EgLab::Common::Matrix4f::Identity();
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i == j)
                EXPECT_EQ(mat[i][j], 1);
            else
                EXPECT_EQ(mat[i][j], 0);
        }
    }
}

#if 0
#include <iostream>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <fstream>
#include <string>
#endif

void printCacheSize() {
#if defined(_WIN32)
    // Windows平台使用Windows API查�?�缓存信�?
    DWORD bufferSize = 0;
    GetLogicalProcessorInformation(nullptr, &bufferSize);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(bufferSize);
    if (!buffer) {
        std::cerr << "内存分配失败\n";
        return;
    }
    if (!GetLogicalProcessorInformation(buffer, &bufferSize)) {
        std::cerr << "获取处理器信�?失败\n";
        free(buffer);
        return;
    }

    size_t count = bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    for (size_t i = 0; i < count; ++i) {
        if (buffer[i].Relationship == RelationCache) {
            CACHE_DESCRIPTOR cache = buffer[i].Cache;
            std::string level = "L" + std::to_string(cache.Level);
            std::string type;
            switch (cache.Type) {
                case CacheUnified: type = "Unified"; break;
                case CacheInstruction: type = "Instruction"; break;
                case CacheData: type = "Data"; break;
                default: type = "Unknown"; break;
            }
            std::cout << level << " Cache: " << cache.Size / 1024 << " KB, Type: " << type << std::endl;
        }
    }
    free(buffer);

#elif defined(__linux__)
    // Linux平台读取/sys/devices/system/cpu/cpu0/cache/index*/size
    for (int i = 0; i < 4; ++i) {
        std::string path = "/sys/devices/system/cpu/cpu0/cache/index" + std::to_string(i) + "/size";
        std::ifstream file(path);
        if (file) {
            std::string sizeStr;
            std::getline(file, sizeStr);
            std::cout << "Cache index" << i << " size: " << sizeStr << std::endl;
        }
    }
#else
    std::cout << "当前平台暂不�?持自动�?�测缓存大小\n";
#endif
}

TEST_F(TestCommon, testCache)
{
    printCacheSize();
}
#endif

TEST_F(TestCommon, MemoryQueue)
{
    const uint32_t channelId = 101;
    EgLab::Common::String payload = "Hello MemoryQueue";

    EgLab::Common::String shmName = "TestMemoryQueue";

    // 初始化队列，true 表示如果不存在则创建
    auto queue = EgLab::Common::makeUnique<EgLab::Common::MemoryQueue>(shmName, true);

    // 写入数据
    bool pushResult = queue->push(channelId, payload.c_str(), payload.size());
    EXPECT_TRUE(pushResult) << "Push Operation Succeed";

    // 读取数据
    uint32_t outChannelId = 0;
    char buffer[256] = {0};
    uint32_t actualSize = 0;

    bool popResult = queue->pop(outChannelId, buffer, sizeof(buffer), actualSize);

    EXPECT_TRUE(popResult) << "Pop Operation Succed";
    EXPECT_EQ(outChannelId, channelId) << "ChannelId Same";
    EXPECT_EQ(actualSize, payload.size()) << "data size";
    EXPECT_STREQ(buffer, payload.c_str()) << "data size";
}

TEST_F(TestCommon, MemoryQueue_ConcurrentStress)
{
    const int kMessageCount = 10000;
    std::atomic<int> consumedCount(0);
    // const uint32_t channelId = 101;
    EgLab::Common::String payload = "Hello MemoryQueue";

    EgLab::Common::String shmName = "TestMemoryQueue";
    auto queue = EgLab::Common::makeUnique<EgLab::Common::MemoryQueue>(shmName, true);

    // 启动消费者线程
    std::thread consumer([&queue, &consumedCount]() {
        char buffer[256];
        uint32_t outChannelId;
        uint32_t actualSize;

        while (consumedCount.load() < kMessageCount)
        {
            if (queue->pop(outChannelId, buffer, sizeof(buffer), actualSize))
            {
                consumedCount++;
            }
        }
    });

    // 生产者（当前线程）全速写入
    for (int i = 0; i < kMessageCount; ++i)
    {
        while (!queue->push(1, &i, sizeof(i)))
        {
            // 队列满时自旋等待
            std::this_thread::yield();
        }
    }

    consumer.join();
    EXPECT_EQ(consumedCount.load(), kMessageCount) << "消费者应该收到所有消息";
}

class TestPubSub : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 使用带时间戳的唯一名称，确保高并发执行测试套件时不会发生命名冲突
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        _shmName = "TestPubSub_" + EgLab::Common::String(std::to_string(now).c_str());

        // 初始化底层共享内存队列
        _queue = EgLab::Common::makeUnique<EgLab::Common::MemoryQueue>(_shmName, true);
    }

    void TearDown() override
    {
        // 释放当前进程对共享内存的映射
        _queue = nullptr;

        // 注意：如果是 POSIX 环境，这里通常需要调用 shm_unlink(_shmName.c_str()) 来彻底清理。
    }

    EgLab::Common::UniquePtr<EgLab::Common::MemoryQueue> _queue;
    EgLab::Common::String _shmName;
};

struct MeshData
{
    uint32_t meshId;
    float qualityScore;
};

// 2. 多线程测试：验证同一进程内，多线程并发读写的事件路由正确性
TEST_F(TestPubSub, MultiThreadedPubSub)
{
    const int kMessageCount = 10000;
    std::atomic<int> consumedCount(0);

    EgLab::Common::Subscriber subscriber(*_queue);
    subscriber.subscrib(101);

    std::thread consumerThread([&]() {
        MeshData receivedData;
        while (consumedCount.load() < kMessageCount)
        {
            uint32_t eventId;
            if (subscriber.tryReceive(eventId, receivedData))
            {
                EXPECT_EQ(eventId, 101);
                consumedCount++;
            }
        }
    });

    EgLab::Common::Publisher publisher(*_queue);

    for (int i = 0; i < kMessageCount; ++i)
    {
        MeshData dataToSend{static_cast<uint32_t>(i), 0.95f};
        while (!publisher.publish(101, dataToSend))
        {
            std::this_thread::yield();
        }
    }

    consumerThread.join();
    EXPECT_EQ(consumedCount.load(), kMessageCount) << "消费者应该收到所有消息";
}

TEST_F(TestPubSub, basic)
{
    EgLab::Common::MemoryQueue queue("GlobalEventQueue", true);

    EgLab::Common::Publisher publisher(queue);

    EgLab::Common::Subscriber subscribe(queue);
    subscribe.subscrib(1);

    int a = 4;
    int b = 0;

    publisher.publish(1, a);

    uint32_t eventId;
    if (subscribe.tryReceive(eventId, b))
    {
        EXPECT_EQ(eventId, 1);
        EXPECT_EQ(a, b);
    }

    MeshData ma, mb;
    ma.meshId = 4;
    ma.qualityScore = 4.56;

    publisher.publish(1, ma);
    if (subscribe.tryReceive(eventId, mb))
    {
        EXPECT_EQ(eventId, 1);
        EXPECT_EQ(ma.meshId, mb.meshId);
        EXPECT_EQ(ma.qualityScore, mb.qualityScore);
    }
}

TEST_F(TestHashMap, clear)
{
    EgLab::Common::HashMap<int, int> map;
    map[1] = 2;
    map[2] = 3;
    EXPECT_EQ(map.size(), 2);

    map.clear();

    EXPECT_EQ(map.size(), 0);
    EXPECT_TRUE(map.empty());
}

TEST_F(TestHashMap, BigData)
{
    EgLab::Common::HashMap<int, int> map;
    map[100000] = 2;
    map[2] = 3;
    EXPECT_EQ(map.size(), 2);

    map.clear();

    EXPECT_EQ(map.size(), 0);
    EXPECT_TRUE(map.empty());
}

TEST_F(TestHashMap, Reserve)
{
    EgLab::Common::HashMap<int, int> map;

    map.reserve(64);
    EXPECT_GE(map.bucketsSize(), 64);
    EXPECT_EQ(map.size(), 0);

    for (int i = 0; i < 64; ++i)
    {
        map.insert({i, i * 2});
    }

    EXPECT_EQ(map.size(), 64);
    for (int i = 0; i < 64; ++i)
    {
        EXPECT_EQ(map[i], i * 2);
    }

    const auto bucketCount = map.bucketsSize();
    map.reserve(8);
    EXPECT_EQ(map.bucketsSize(), bucketCount);
    EXPECT_EQ(map.size(), 64);
}

TEST_F(TestList, clear)
{
    EgLab::Common::List<int> l;
    l.pushBack(1);
    l.pushBack(1);
    l.pushBack(1);
    l.pushBack(1);

    EXPECT_EQ(l.size(), 4);
    auto newL = l;
    EXPECT_EQ(newL.size(), 4);
    EXPECT_EQ(newL.begin().data(), 1);
    EgLab::Common::List<int> newL2(l);
    EXPECT_EQ(newL2.size(), 4);
    //  l.clear();
}

TEST_F(TestCommon, DynamicArrayTest)
{
    EgLab::Common::DynamicArray<int> arr;
    EXPECT_EQ(arr.size(), 0);
    EXPECT_EQ(arr.capacity(), 0);
    EXPECT_TRUE(arr.empty());

    int a = 1;
    arr.pushBack(a);
    EXPECT_EQ(arr.size(), 1);
    EXPECT_EQ(arr.capacity(), 2); // 默�?�初始�?�量�?0，�?�加�?2

    int b = 2;
    arr.pushBack(b);
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr.capacity(), 2); // 容量不变

    int c = 3;
    arr.pushBack(c);
    EXPECT_EQ(arr.size(), 3);
    EXPECT_GT(arr.capacity(), 2); // 容量应�?��?�加

    EXPECT_EQ(arr[2], 3);

    for (int i = 0; i < arr.size(); i++)
    {
        LOG(INFO) << arr[i];
    }

    class AAA
    {
    public:
        int x;
        int y;
        int z;
    };

    EgLab::Common::DynamicArray<AAA> arr2;
    arr2.resize(1000);

    for (int i = 0; i < arr2.size(); i++)
    {
        arr2[i].x = i;
        arr2[i].y = i * 2;
        arr2[i].z = i * 3;
    }

    for (int i = 0; i < arr2.size(); i++)
    {
        LOG(INFO) << "index :" << i << " x : " << arr2[i].x << " y : " << arr2[i].y
                  << " z : " << arr2[i].z;
    }
}

TEST_F(TestCommon, DynamicArrayReserve)
{
    EgLab::Common::DynamicArray<int> arr;

    arr.reserve(32);
    EXPECT_GE(arr.capacity(), 32);
    EXPECT_TRUE(arr.empty());

    for (int i = 0; i < 32; ++i)
    {
        arr.pushBack(i);
    }

    EXPECT_EQ(arr.size(), 32);
    for (int i = 0; i < 32; ++i)
    {
        EXPECT_EQ(arr[i], i);
    }

    const auto capacity = arr.capacity();
    arr.reserve(8);
    EXPECT_EQ(arr.capacity(), capacity);
    EXPECT_EQ(arr.size(), 32);
}