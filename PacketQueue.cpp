// #include "PacketQueue.h"

// void PacketQueue::push(const FramePacket& data)
// {
//     QMutexLocker locker(&mutex);
//     queue.enqueue(data);
//     cond.wakeOne();
// }

// FramePacket PacketQueue::pop()
// {
//     QMutexLocker locker(&mutex);

//     while (queue.isEmpty())
//         cond.wait(&mutex);

//     return queue.dequeue();
// }

// int PacketQueue::size()
// {
//     QMutexLocker locker(&mutex);
//     return queue.size();
// }
