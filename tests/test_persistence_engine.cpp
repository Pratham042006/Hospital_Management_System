#include "hms/persistence_engine.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

void testPersistenceEngineOperations() {
    const std::string testDir = "test_data_dir";
    hms::PersistenceEngine engine(testDir);

    // Save records
    bool s1 = engine.saveRecord("patients", "P-00001", "Emily Rose|1992-04-12|Female");
    bool s2 = engine.saveRecord("patients", "P-00002", "Robert Paulson|1980-01-15|Male");
    assert(s1 && s2);

    // Load records
    auto loaded = engine.loadAllRecords("patients");
    assert(loaded.size() == 2);
    assert(loaded[0].find("P-00001") != std::string::npos);
    assert(loaded[1].find("P-00002") != std::string::npos);

    // Non-existent entity returns empty
    auto emptyList = engine.loadAllRecords("non_existent_entity");
    assert(emptyList.empty());

    // Clean up
    engine.clearStorage("patients");
    auto afterClear = engine.loadAllRecords("patients");
    assert(afterClear.empty());

    std::filesystem::remove_all(testDir);
    std::cout << "testPersistenceEngineOperations passed\n";
}

int main() {
    testPersistenceEngineOperations();
    std::cout << "All persistence engine tests passed successfully.\n";
    return 0;
}
