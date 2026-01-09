#include <gtest/gtest.h>
#include "package.hpp"
#include "storage_types.hpp"
#include "types.hpp"

TEST(PackageTest, IsAssignedIdLowest) {
    // przydzielanie ID o jeden większych -- utworzenie dwóch obiektów pod rząd

    Package p1;
    Package p2;

    EXPECT_EQ(p1.get_id(), 1);
    EXPECT_EQ(p2.get_id(), 2);
}

TEST(PackageTest, IsIdReused) {
    // przydzielanie ID po zwolnionym obiekcie

    {
        Package p1;
    }
    Package p2;

    EXPECT_EQ(p2.get_id(), 1);
}

TEST(PackageTest, IsMoveConstructorCorrect) {
    Package p1;
    Package p2(std::move(p1));

    EXPECT_EQ(p2.get_id(), 1);
}

TEST(PackageTest, IsAssignmentOperatorCorrect) {
    Package p1;
    Package p2 = std::move(p1);

    EXPECT_EQ(p2.get_id(), 1);
}

TEST(PackageTest, IsIdNotDuplicatedAfterMove) {
    Package p1;
    Package p2 = std::move(p1);
    Package p3;

    EXPECT_EQ(p2.get_id(), 1);
    EXPECT_EQ(p3.get_id(), 2);
}


TEST(PackageQueueTest, IsFifoCorrect) {
    PackageQueue q(PackageQueueType::FIFO);
    q.push(Package(1));
    q.push(Package(2));

    Package p(std::move(q.pop()));
    EXPECT_EQ(p.get_id(), 1);

    p = q.pop();
    EXPECT_EQ(p.get_id(), 2);
}

TEST(PackageQueueTest, IsLifoCorrect) {
    PackageQueue q(PackageQueueType::LIFO);
    q.push(Package(1));
    q.push(Package(2));

    Package p(std::move(q.pop()));
    EXPECT_EQ(p.get_id(), 2);

    p = q.pop();
    EXPECT_EQ(p.get_id(), 1);
}
TEST(PackageQueueTest, IsEmptyCorrect) {
    PackageQueue q(PackageQueueType::FIFO);
    EXPECT_TRUE(q.empty());

    q.push(Package(1));
    EXPECT_FALSE(q.empty());

    q.pop();
    EXPECT_TRUE(q.empty());
}
TEST(PackageQueueTest, IsSizeCorrect) {
    PackageQueue q(PackageQueueType::FIFO);
    EXPECT_EQ(q.size(), 0);

    q.push(Package(1));
    EXPECT_EQ(q.size(), 1);

    q.push(Package(2));
    EXPECT_EQ(q.size(), 2);

    q.pop();
    EXPECT_EQ(q.size(), 1);

    q.pop();
    EXPECT_EQ(q.size(), 0);
}
TEST(PackageQueueTest, AreIteratorsCorrect) {
    PackageQueue q(PackageQueueType::FIFO);
    q.push(Package(1));
    q.push(Package(2));
    q.push(Package(3));

    int expected_id = 1;
    for (auto it = q.begin(); it != q.end(); ++it) {
        EXPECT_EQ(it->get_id(), expected_id);
        expected_id++;
    }

    expected_id = 1;
    for (auto it = q.cbegin(); it != q.cend(); ++it) {
        EXPECT_EQ(it->get_id(), expected_id);
        expected_id++;
    }
}
