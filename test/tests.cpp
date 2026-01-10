#include <gtest/gtest.h>
#include "package.hpp"
#include "storage_types.hpp"
#include "types.hpp"
#include "nodes.hpp"
#include "helpers.hpp"
#include "factory.hpp"

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

TEST(WorkerTest, SendsProcessedPackageCorrectly) {
    ElementID id = 1;
    TimeOffset processing_duration = 2;
    auto queue = std::make_unique<PackageQueue>(PackageQueueType::FIFO);
    Worker w(id, processing_duration,  std::move(queue));
    Storehouse sh(1, std::make_unique<PackageQueue>(PackageQueueType::FIFO));
    w.receiver_preferences_.add_receiver(&sh);

    Package p1(1);
    Package p2(2);
    w.receive_package(std::move(p1));
    EXPECT_EQ(std::distance(w.cbegin(), w.cend()), 1);
    
    w.receive_package(std::move(p2));
    EXPECT_EQ(std::distance(w.cbegin(), w.cend()), 2);

    w.do_work(1);
    EXPECT_TRUE(w.get_sending_buffer().has_value());

    w.do_work(2);
    EXPECT_FALSE(w.get_sending_buffer().has_value());
    EXPECT_EQ(std::distance(sh.cbegin(), sh.cend()), 1);

    w.do_work(3);
    EXPECT_TRUE(w.get_sending_buffer().has_value());

    w.do_work(4);
    EXPECT_FALSE(w.get_sending_buffer().has_value());
    EXPECT_EQ(std::distance(sh.cbegin(), sh.cend()), 2);
}

TEST(FactoryConsistencyTest, TrivialConsistentNetwork) {
    Factory factory;

    // 1. Tworzenie węzłów
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_storehouse(Storehouse(1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));

    // 2. Pobieranie wskaźników (iteratorów) do obiektów wewnątrz fabryki
    // Uwaga: Musimy to robić PO dodaniu do fabryki, bo przy dodawaniu obiekty są przenoszone (move)
    auto ramp_it = factory.find_ramp_by_id(1);
    auto worker_it = factory.find_worker_by_id(1);
    auto store_it = factory.find_storehouse_by_id(1);

    // 3. Łączenie węzłów (Wiring)
    // Rampa -> Robotnik
    ramp_it->receiver_preferences_.add_receiver(&(*worker_it));
    // Robotnik -> Magazyn
    worker_it->receiver_preferences_.add_receiver(&(*store_it));

    // 4. Weryfikacja
    // Oczekujemy, że sieć jest spójna
    EXPECT_TRUE(factory.is_consistent());
}

// TEST 2: Ślepy zaułek
// Struktura: [Rampa 1] -> [Robotnik 1] -> (BRAK ODBIORCY)
TEST(FactoryConsistencyTest, DeadEndNetwork) {
    Factory factory;

    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    // Brak magazynu!

    auto ramp_it = factory.find_ramp_by_id(1);
    auto worker_it = factory.find_worker_by_id(1);

    // Rampa wysyła do robotnika
    ramp_it->receiver_preferences_.add_receiver(&(*worker_it));
    
    // Robotnik nie wysyła nigdzie (brak add_receiver)

    // Oczekujemy, że sieć NIE jest spójna (is_consistent zwróci false)
    EXPECT_FALSE(factory.is_consistent());
}

// TEST 3: Cykl bez wyjścia
// Struktura: [Rampa 1] -> [Robotnik 1] <--> [Robotnik 2]
// Paczka wpada w pętlę i nigdy nie trafia do magazynu.
TEST(FactoryConsistencyTest, CycleWithoutStorehouse) {
    Factory factory;

    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_worker(Worker(2, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));

    auto ramp_it = factory.find_ramp_by_id(1);
    auto w1_it = factory.find_worker_by_id(1);
    auto w2_it = factory.find_worker_by_id(2);

    // Rampa -> W1
    ramp_it->receiver_preferences_.add_receiver(&(*w1_it));
    
    // W1 -> W2
    w1_it->receiver_preferences_.add_receiver(&(*w2_it));

    // W2 -> W1 (Pętla zamknięta)
    w2_it->receiver_preferences_.add_receiver(&(*w1_it));

    // Oczekujemy false, bo nie ma dojścia do żadnego Storehouse
    EXPECT_FALSE(factory.is_consistent());
}