// Agency output of .[0].arango.Plan.Collections
std::shared_ptr<VPackBuffer<uint8_t>> planCollections = R"=(
{
  "someDb": {
    "10000001": {
      "name": "prototype",
      "replicationFactor": 2,
      "shards": {
        "s11": [
          "PRMR-AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA",
          "PRMR-BBBBBBBB-BBBB-BBBB-BBBB-BBBBBBBBBBBB"
        ]
      }
    },
    "10000002": {
      "name": "follower",
      "replicationFactor": 4,
      "distributeShardsLike": "10000001",
      "shards": {
        "s21": [
          "PRMR-AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA",
          "PRMR-CCCCCCCC-CCCC-CCCC-CCCC-CCCCCCCCCCCC"
        ]
      }
    }
  }
}
)="_vpack;

// Agency output of .[0].arango.Supervision.Health
// Coordinators are unused in the test, but must be ignored
std::shared_ptr<VPackBuffer<uint8_t>> supervisionHealth3Healthy0Bad = R"=(
{
  "CRDN-976e3d6a-9148-4ece-99e9-326dc69834b2": {
  },
  "PRMR-AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA": {
    "Status": "GOOD"
  },
  "CRDN-94ea8912-ff22-43d0-a005-bfc87f22709b": {
  },
  "CRDN-34b46cab-6f06-40a8-ac24-5eec1cf78f67": {
  },
  "PRMR-BBBBBBBB-BBBB-BBBB-BBBB-BBBBBBBBBBBB": {
    "Status": "GOOD"
  },
  "PRMR-CCCCCCCC-CCCC-CCCC-CCCC-CCCCCCCCCCCC": {
    "Status": "GOOD"
  }
}
)="_vpack;

std::map<CollectionID, ResultT<std::vector<RepairOperation>>>
    expectedResultsWithUnequalReplicationFactor{
        {"10000002",
         {{
             // rename distributeShardsLike to repairingDistributeShardsLike
             BeginRepairsOperation{
                 .database = "someDb",
                 .collectionId = "10000002",
                 .collectionName = "follower",
                 .protoCollectionId = "10000001",
                 .protoCollectionName = "prototype",
                 .collectionReplicationFactor = 4,
                 .protoReplicationFactor = 2,
                 .renameDistributeShardsLike = true,
             },
             // shard s21 of collection 10000002
             // move follower
             MoveShardOperation{
                 .database = "someDb",
                 .collectionId = "10000002",
                 .collectionName = "follower",
                 .shard = "s21",
                 .from = "PRMR-CCCCCCCC-CCCC-CCCC-CCCC-CCCCCCCCCCCC",
                 .to = "PRMR-BBBBBBBB-BBBB-BBBB-BBBB-BBBBBBBBBBBB",
                 .isLeader = false,
             },
             // rename repairingDistributeShardsLike to distributeShardsLike
             FinishRepairsOperation{
                 .database = "someDb",
                 .collectionId = "10000002",
                 .collectionName = "follower",
                 .protoCollectionId = "10000001",
                 .protoCollectionName = "prototype",
                 .shards =
                     {
                         std::make_tuple<ShardID, ShardID, DBServers>(
                             "s21", "s11",
                             {
                                 "PRMR-AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAA",
                                 "PRMR-BBBBBBBB-BBBB-BBBB-BBBB-BBBBBBBBBBBB",
                             }),
                     },
                 .replicationFactor = 2,
             },
         }}},
    };
