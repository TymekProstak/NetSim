#include <gtest/gtest.h>
#include "package.hpp"
#include "storage_types.hpp"
#include "types.hpp"
#include "nodes.hpp"
#include "helpers.hpp"

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

TEST(PackageSenderTest, SendingClearsBuffer) {
    Ramp r(1,1);
    Storehouse sh(1, std::make_unique<PackageQueue>(PackageQueueType::FIFO));
    r.receiver_preferences_.add_receiver(&sh);
    r.deliver_goods(1);
    
    ASSERT_TRUE(r.get_sending_buffer().has_value());

    r.send_package();

    EXPECT_FALSE(r.get_sending_buffer().has_value());

    EXPECT_EQ(std::distance(sh.cbegin(), sh.cend()), 1);
}

TEST(ReceiverPreferencesTest, AddingReceiversProbabilities) {
    ReceiverPreferences rp;
    Storehouse sh1(1, std::make_unique<PackageQueue>(PackageQueueType::FIFO));
    Storehouse sh2(2, std::make_unique<PackageQueue>(PackageQueueType::FIFO));
    Storehouse sh3(3, std::make_unique<PackageQueue>(PackageQueueType::FIFO));

    rp.add_receiver(&sh1);
    ASSERT_EQ(rp.get_preferences().size(), 1);
    EXPECT_DOUBLE_EQ(rp.get_preferences().at(&sh1), 1.0);

    rp.add_receiver(&sh2);
    ASSERT_EQ(rp.get_preferences().size(), 2);
    EXPECT_DOUBLE_EQ(rp.get_preferences().at(&sh2), 0.5);
    EXPECT_DOUBLE_EQ(rp.get_preferences().at(&sh1), 1);

    rp.add_receiver(&sh3);
    ASSERT_EQ(rp.get_preferences().size(), 3);
    EXPECT_DOUBLE_EQ(rp.get_preferences().at(&sh3), 1.0/3.0);
    EXPECT_DOUBLE_EQ(rp.get_preferences().at(&sh2), 2.0/3.0);
    EXPECT_DOUBLE_EQ(rp.get_preferences().at(&sh1), 3.0/3.0);   
} 

TEST(ReceiverPreferencesTest, RemovingReceiversProbabilities) {
    ReceiverPreferences rp;
    Storehouse sh1(1, std::make_unique<PackageQueue>(PackageQueueType::FIFO));
    Storehouse sh2(2, std::make_unique<PackageQueue>(PackageQueueType::FIFO));
    Storehouse sh3(3, std::make_unique<PackageQueue>(PackageQueueType::FIFO));

    rp.add_receiver(&sh1);
    rp.add_receiver(&sh2);
    rp.add_receiver(&sh3);

    rp.remove_receiver(&sh1);
    ASSERT_EQ(rp.get_preferences().size(), 2);
    EXPECT_DOUBLE_EQ(rp.get_preferences().at(&sh3), 0.5);
    EXPECT_DOUBLE_EQ(rp.get_preferences().at(&sh2), 1.0);

    rp.remove_receiver(&sh2);
    ASSERT_EQ(rp.get_preferences().size(), 1);
    EXPECT_DOUBLE_EQ(rp.get_preferences().at(&sh3), 1.0);

    rp.remove_receiver(&sh3);
    ASSERT_EQ(rp.get_preferences().size(), 0);
}

TEST(ReceiverPreferencesTest, ChooseReceiverProbability) {
    Storehouse sh1(1, std::make_unique<PackageQueue>(PackageQueueType::FIFO));
    Storehouse sh2(2, std::make_unique<PackageQueue>(PackageQueueType::FIFO));
    Storehouse sh3(3, std::make_unique<PackageQueue>(PackageQueueType::FIFO));

    std::vector<double> probabilities = {0.3, 0.5, 0.67};
    size_t count = 0;
    
    auto mock_gen = [&count, &probabilities]() -> double {
        return probabilities[count++];
    };
    
    ReceiverPreferences rp(mock_gen);
    rp.add_receiver(&sh1);
    rp.add_receiver(&sh2);
    rp.add_receiver(&sh3);

    EXPECT_EQ(rp.choose_receiver(), &sh3);
    EXPECT_EQ(rp.choose_receiver(), &sh2);
    EXPECT_EQ(rp.choose_receiver(), &sh1); 
}

TEST(RampTest, IsTheDeliveryHappeningInTheCorrectTurn) {
    ElementID id = 1;
    TimeOffset di = 3;
    Ramp r(id, di);
    Storehouse sh(1, std::make_unique<PackageQueue>(PackageQueueType::FIFO));
    r.receiver_preferences_.add_receiver(&sh);

    EXPECT_FALSE(r.get_sending_buffer().has_value());

    r.deliver_goods(1);
    ASSERT_TRUE(r.get_sending_buffer().has_value());

    const std::optional<Package>& first_package = r.get_sending_buffer();
    ElementID fp_id = first_package->get_id();
    

    r.send_package();

    r.deliver_goods(2);
    EXPECT_FALSE(r.get_sending_buffer().has_value());
    
    r.deliver_goods(3);
    EXPECT_FALSE(r.get_sending_buffer().has_value());

    r.deliver_goods(4);
    EXPECT_TRUE(r.get_sending_buffer().has_value());
    EXPECT_NE(r.get_sending_buffer().value(), fp_id);
}

TEST(WorkerTest, ReceivesPackageProperly) {
    ElementID id = 1;
    TimeOffset processing_duration = 2;
    auto queue = std::make_unique<PackageQueue>(PackageQueueType::FIFO);
    Worker w(id, processing_duration,  std::move(queue));

    Package p1(1);
    Package p2(2);
    Package p3(3);
    w.receive_package(std::move(p1));
    EXPECT_EQ(std::distance(w.cbegin(), w.cend()), 1);
    w.receive_package(std::move(p2));
    EXPECT_EQ(std::distance(w.cbegin(), w.cend()), 2);
    w.receive_package(std::move(p3));
    EXPECT_EQ(std::distance(w.cbegin(), w.cend()), 3);
}

TEST(WorkerTest, ProcessesPackageCorrectly) {
    ElementID id = 1;
    TimeOffset processing_duration = 3;
    auto queue = std::make_unique<PackageQueue>(PackageQueueType::FIFO);
    Worker w(id, processing_duration,  std::move(queue));

    Package p1(1);
    Package p2(2);
    w.receive_package(std::move(p1));
    w.receive_package(std::move(p2));
    EXPECT_FALSE(w.get_sending_buffer().has_value());

    w.do_work(1);
    // processing starts
    EXPECT_TRUE(w.get_sending_buffer().has_value());

    w.do_work(2);
    // still processing
    EXPECT_TRUE(w.get_sending_buffer().has_value());

    w.do_work(3);
    // processing ends
    EXPECT_FALSE(w.get_sending_buffer().has_value());
    
    w.do_work(4);
    // next package is put into processing
    EXPECT_TRUE(w.get_sending_buffer().has_value());

    w.do_work(5);
    // still processing
    EXPECT_TRUE(w.get_sending_buffer().has_value());

    w.do_work(6);
    // processing ends
    EXPECT_FALSE(w.get_sending_buffer().has_value());
}