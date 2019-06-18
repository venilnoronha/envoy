// NOLINT(namespace-envoy)

// This file is part of the QUICHE platform implementation, and is not to be
// consumed or referenced directly by other Envoy code. It serves purely as a
// porting layer for QUICHE.

#include <fstream>
#include <unordered_set>

#include "test/extensions/transport_sockets/tls/ssl_test_utility.h"
#include "test/test_common/environment.h"
#include "test/test_common/logging.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "quiche/quic/platform/api/quic_aligned.h"
#include "quiche/quic/platform/api/quic_arraysize.h"
#include "quiche/quic/platform/api/quic_bug_tracker.h"
#include "quiche/quic/platform/api/quic_cert_utils.h"
#include "quiche/quic/platform/api/quic_client_stats.h"
#include "quiche/quic/platform/api/quic_containers.h"
#include "quiche/quic/platform/api/quic_endian.h"
#include "quiche/quic/platform/api/quic_estimate_memory_usage.h"
#include "quiche/quic/platform/api/quic_expect_bug.h"
#include "quiche/quic/platform/api/quic_exported_stats.h"
#include "quiche/quic/platform/api/quic_file_utils.h"
#include "quiche/quic/platform/api/quic_hostname_utils.h"
#include "quiche/quic/platform/api/quic_logging.h"
#include "quiche/quic/platform/api/quic_map_util.h"
#include "quiche/quic/platform/api/quic_mock_log.h"
#include "quiche/quic/platform/api/quic_mutex.h"
#include "quiche/quic/platform/api/quic_ptr_util.h"
#include "quiche/quic/platform/api/quic_server_stats.h"
#include "quiche/quic/platform/api/quic_sleep.h"
#include "quiche/quic/platform/api/quic_stack_trace.h"
#include "quiche/quic/platform/api/quic_string_piece.h"
#include "quiche/quic/platform/api/quic_test_output.h"
#include "quiche/quic/platform/api/quic_thread.h"
#include "quiche/quic/platform/api/quic_uint128.h"

using testing::HasSubstr;

// Basic tests to validate functioning of the QUICHE quic platform
// implementation. For platform APIs in which the implementation is a simple
// typedef/passthrough to a std:: or absl:: construct, the tests are kept
// minimal, and serve primarily to verify the APIs compile and link without
// issue.

namespace quic {
namespace {

class QuicPlatformTest : public testing::Test {
protected:
  QuicPlatformTest()
      : log_level_(GetLogger().level()), verbosity_log_threshold_(GetVerbosityLogThreshold()) {
    SetVerbosityLogThreshold(0);
    GetLogger().set_level(ERROR);
  }

  ~QuicPlatformTest() {
    SetVerbosityLogThreshold(verbosity_log_threshold_);
    GetLogger().set_level(log_level_);
  }

  const QuicLogLevel log_level_;
  const int verbosity_log_threshold_;
};

TEST_F(QuicPlatformTest, QuicAlignOf) { EXPECT_LT(0, QUIC_ALIGN_OF(int)); }

TEST_F(QuicPlatformTest, QuicArraysize) {
  int array[] = {0, 1, 2, 3, 4};
  EXPECT_EQ(5, QUIC_ARRAYSIZE(array));
}

enum class TestEnum { ZERO = 0, ONE, TWO, COUNT };

TEST_F(QuicPlatformTest, QuicBugTracker) {
  EXPECT_DEBUG_DEATH(QUIC_BUG << "Here is a bug,", " bug");
  EXPECT_DEBUG_DEATH(QUIC_BUG_IF(true) << "There is a bug,", " bug");
  EXPECT_LOG_NOT_CONTAINS("error", "", QUIC_BUG_IF(false) << "A feature is not a bug.");

  EXPECT_LOG_CONTAINS("error", " bug", QUIC_PEER_BUG << "Everywhere's a bug,");
  EXPECT_LOG_CONTAINS("error", " here", QUIC_PEER_BUG_IF(true) << "Including here.");
  EXPECT_LOG_NOT_CONTAINS("error", "", QUIC_PEER_BUG_IF(false) << "But not there.");
}

TEST_F(QuicPlatformTest, QuicClientStats) {
  // Just make sure they compile.
  QUIC_CLIENT_HISTOGRAM_ENUM("my.enum.histogram", TestEnum::ONE, TestEnum::COUNT, "doc");
  QUIC_CLIENT_HISTOGRAM_BOOL("my.bool.histogram", false, "doc");
  QUIC_CLIENT_HISTOGRAM_TIMES("my.timing.histogram", QuicTime::Delta::FromSeconds(5),
                              QuicTime::Delta::FromSeconds(1), QuicTime::Delta::FromSecond(3600),
                              100, "doc");
  QUIC_CLIENT_HISTOGRAM_COUNTS("my.count.histogram", 123, 0, 1000, 100, "doc");
  QuicClientSparseHistogram("my.sparse.histogram", 345);
}

TEST_F(QuicPlatformTest, QuicExpectBug) {
  auto bug = [](const char* error_message) { QUIC_BUG << error_message; };

  auto peer_bug = [](const char* error_message) { QUIC_PEER_BUG << error_message; };

  EXPECT_QUIC_BUG(bug("bug one is expected"), "bug one");
  EXPECT_QUIC_BUG(bug("bug two is expected"), "bug two");

  EXPECT_QUIC_PEER_BUG(peer_bug("peer_bug_1 is expected"), "peer_bug_1");
  EXPECT_QUIC_PEER_BUG(peer_bug("peer_bug_2 is expected"), "peer_bug_2");
}

TEST_F(QuicPlatformTest, QuicExportedStats) {
  // Just make sure they compile.
  QUIC_HISTOGRAM_ENUM("my.enum.histogram", TestEnum::ONE, TestEnum::COUNT, "doc");
  QUIC_HISTOGRAM_BOOL("my.bool.histogram", false, "doc");
  QUIC_HISTOGRAM_TIMES("my.timing.histogram", QuicTime::Delta::FromSeconds(5),
                       QuicTime::Delta::FromSeconds(1), QuicTime::Delta::FromSecond(3600), 100,
                       "doc");
  QUIC_HISTOGRAM_COUNTS("my.count.histogram", 123, 0, 1000, 100, "doc");
}

TEST_F(QuicPlatformTest, QuicHostnameUtils) {
  EXPECT_FALSE(QuicHostnameUtils::IsValidSNI("!!"));
  EXPECT_FALSE(QuicHostnameUtils::IsValidSNI("envoyproxy"));
  EXPECT_TRUE(QuicHostnameUtils::IsValidSNI("www.envoyproxy.io"));
  EXPECT_EQ("lyft.com", QuicHostnameUtils::NormalizeHostname("lyft.com"));
  EXPECT_EQ("google.com", QuicHostnameUtils::NormalizeHostname("google.com..."));
  EXPECT_EQ("quicwg.org", QuicHostnameUtils::NormalizeHostname("QUICWG.ORG"));
}

TEST_F(QuicPlatformTest, QuicUnorderedMap) {
  QuicUnorderedMap<std::string, int> umap;
  umap.insert({"foo", 2});
  EXPECT_EQ(2, umap["foo"]);
}

TEST_F(QuicPlatformTest, QuicUnorderedSet) {
  QuicUnorderedSet<std::string> uset({"foo", "bar"});
  EXPECT_EQ(1, uset.count("bar"));
  EXPECT_EQ(0, uset.count("qux"));
}

TEST_F(QuicPlatformTest, QuicQueue) {
  QuicQueue<int> queue;
  queue.push(10);
  EXPECT_EQ(10, queue.back());
}

TEST_F(QuicPlatformTest, QuicDeque) {
  QuicDeque<int> deque;
  deque.push_back(10);
  EXPECT_EQ(10, deque.back());
}

TEST_F(QuicPlatformTest, QuicInlinedVector) {
  QuicInlinedVector<int, 5> vec;
  vec.push_back(3);
  EXPECT_EQ(3, vec[0]);
}

TEST_F(QuicPlatformTest, QuicEndian) {
  EXPECT_EQ(0x1234, QuicEndian::NetToHost16(QuicEndian::HostToNet16(0x1234)));
  EXPECT_EQ(0x12345678, QuicEndian::NetToHost32(QuicEndian::HostToNet32(0x12345678)));
}

TEST_F(QuicPlatformTest, QuicEstimateMemoryUsage) {
  std::string s = "foo";
  // Stubbed out to always return 0.
  EXPECT_EQ(0, QuicEstimateMemoryUsage(s));
}

TEST_F(QuicPlatformTest, QuicMapUtil) {
  std::map<std::string, int> stdmap = {{"one", 1}, {"two", 2}, {"three", 3}};
  EXPECT_TRUE(QuicContainsKey(stdmap, "one"));
  EXPECT_FALSE(QuicContainsKey(stdmap, "zero"));

  QuicUnorderedMap<int, int> umap = {{1, 1}, {2, 4}, {3, 9}};
  EXPECT_TRUE(QuicContainsKey(umap, 2));
  EXPECT_FALSE(QuicContainsKey(umap, 10));

  QuicUnorderedSet<std::string> uset({"foo", "bar"});
  EXPECT_TRUE(QuicContainsKey(uset, "foo"));
  EXPECT_FALSE(QuicContainsKey(uset, "abc"));

  std::vector<int> stdvec = {1, 2, 3};
  EXPECT_TRUE(QuicContainsValue(stdvec, 1));
  EXPECT_FALSE(QuicContainsValue(stdvec, 0));
}

TEST_F(QuicPlatformTest, QuicMockLog) {
  ASSERT_EQ(ERROR, GetLogger().level());

  {
    // Test a mock log that is not capturing logs.
    CREATE_QUIC_MOCK_LOG(log);
    EXPECT_QUIC_LOG_CALL(log).Times(0);
    QUIC_LOG(ERROR) << "This should be logged but not captured by the mock.";
  }

  // Test nested mock logs.
  CREATE_QUIC_MOCK_LOG(outer_log);
  outer_log.StartCapturingLogs();

  {
    // Test a mock log that captures logs.
    CREATE_QUIC_MOCK_LOG(inner_log);
    inner_log.StartCapturingLogs();

    EXPECT_QUIC_LOG_CALL_CONTAINS(inner_log, ERROR, "Inner log message");
    QUIC_LOG(ERROR) << "Inner log message should be captured.";

    // Destruction of inner_log should restore the QUIC log sink to outer_log.
  }

  EXPECT_QUIC_LOG_CALL_CONTAINS(outer_log, ERROR, "Outer log message");
  QUIC_LOG(ERROR) << "Outer log message should be captured.";
}

TEST_F(QuicPlatformTest, QuicServerStats) {
  // Just make sure they compile.
  QUIC_SERVER_HISTOGRAM_ENUM("my.enum.histogram", TestEnum::ONE, TestEnum::COUNT, "doc");
  QUIC_SERVER_HISTOGRAM_BOOL("my.bool.histogram", false, "doc");
  QUIC_SERVER_HISTOGRAM_TIMES("my.timing.histogram", QuicTime::Delta::FromSeconds(5),
                              QuicTime::Delta::FromSeconds(1), QuicTime::Delta::FromSecond(3600),
                              100, "doc");
  QUIC_SERVER_HISTOGRAM_COUNTS("my.count.histogram", 123, 0, 1000, 100, "doc");
}

TEST_F(QuicPlatformTest, DISABLED_QuicStackTraceTest) {
  EXPECT_THAT(QuicStackTrace(), HasSubstr("QuicStackTraceTest"));
}

TEST_F(QuicPlatformTest, QuicSleep) { QuicSleep(QuicTime::Delta::FromMilliseconds(20)); }

TEST_F(QuicPlatformTest, QuicStringPiece) {
  std::string s = "bar";
  QuicStringPiece sp(s);
  EXPECT_EQ('b', sp[0]);
}

TEST_F(QuicPlatformTest, QuicThread) {
  class AdderThread : public QuicThread {
  public:
    AdderThread(int* value, int increment)
        : QuicThread("adder_thread"), value_(value), increment_(increment) {}

    ~AdderThread() override = default;

  protected:
    void Run() override { *value_ += increment_; }

  private:
    int* value_;
    int increment_;
  };

  int value = 0;

  // A QuicThread that is never started, which is ok.
  { AdderThread t0(&value, 1); }
  EXPECT_EQ(0, value);

  // A QuicThread that is started and joined as usual.
  {
    AdderThread t1(&value, 1);
    t1.Start();
    t1.Join();
  }
  EXPECT_EQ(1, value);

  // QuicThread will panic if it's started but not joined.
  EXPECT_DEATH_LOG_TO_STDERR({ AdderThread(&value, 2).Start(); },
                             "QuicThread should be joined before destruction");
}

TEST_F(QuicPlatformTest, QuicUint128) {
  QuicUint128 i = MakeQuicUint128(16777216, 315);
  EXPECT_EQ(315, QuicUint128Low64(i));
  EXPECT_EQ(16777216, QuicUint128High64(i));
}

TEST_F(QuicPlatformTest, QuicPtrUtil) {
  auto p = QuicMakeUnique<std::string>("abc");
  EXPECT_EQ("abc", *p);

  p = QuicWrapUnique(new std::string("aaa"));
  EXPECT_EQ("aaa", *p);
}

TEST_F(QuicPlatformTest, QuicLog) {
  // By default, tests emit logs at level ERROR or higher.
  ASSERT_EQ(ERROR, GetLogger().level());

  int i = 0;

  QUIC_LOG(INFO) << (i = 10);
  QUIC_LOG_IF(INFO, false) << i++;
  QUIC_LOG_IF(INFO, true) << i++;
  EXPECT_EQ(0, i);

  EXPECT_LOG_CONTAINS("error", "i=11", QUIC_LOG(ERROR) << "i=" << (i = 11));
  EXPECT_EQ(11, i);

  QUIC_LOG_IF(ERROR, false) << i++;
  EXPECT_EQ(11, i);

  EXPECT_LOG_CONTAINS("error", "i=11", QUIC_LOG_IF(ERROR, true) << "i=" << i++);
  EXPECT_EQ(12, i);

  // Set QUIC log level to INFO, since VLOG is emitted at the INFO level.
  GetLogger().set_level(INFO);

  ASSERT_EQ(0, GetVerbosityLogThreshold());

  QUIC_VLOG(1) << (i = 1);
  EXPECT_EQ(12, i);

  SetVerbosityLogThreshold(1);

  EXPECT_LOG_CONTAINS("info", "i=1", QUIC_VLOG(1) << "i=" << (i = 1));
  EXPECT_EQ(1, i);

  errno = EINVAL;
  EXPECT_LOG_CONTAINS("info", "i=3:", QUIC_PLOG(INFO) << "i=" << (i = 3));
  EXPECT_EQ(3, i);
}

#ifdef NDEBUG
#define VALUE_BY_COMPILE_MODE(debug_mode_value, release_mode_value) release_mode_value
#else
#define VALUE_BY_COMPILE_MODE(debug_mode_value, release_mode_value) debug_mode_value
#endif

TEST_F(QuicPlatformTest, QuicDLog) {
  int i = 0;

  GetLogger().set_level(ERROR);

  QUIC_DLOG(INFO) << (i = 10);
  QUIC_DLOG_IF(INFO, false) << i++;
  QUIC_DLOG_IF(INFO, true) << i++;
  EXPECT_EQ(0, i);

  GetLogger().set_level(INFO);

  QUIC_DLOG(INFO) << (i = 10);
  QUIC_DLOG_IF(INFO, false) << i++;
  EXPECT_EQ(VALUE_BY_COMPILE_MODE(10, 0), i);

  QUIC_DLOG_IF(INFO, true) << (i = 11);
  EXPECT_EQ(VALUE_BY_COMPILE_MODE(11, 0), i);

  ASSERT_EQ(0, GetVerbosityLogThreshold());

  QUIC_DVLOG(1) << (i = 1);
  EXPECT_EQ(VALUE_BY_COMPILE_MODE(11, 0), i);

  SetVerbosityLogThreshold(1);

  QUIC_DVLOG(1) << (i = 1);
  EXPECT_EQ(VALUE_BY_COMPILE_MODE(1, 0), i);

  QUIC_DVLOG_IF(1, false) << (i = 2);
  EXPECT_EQ(VALUE_BY_COMPILE_MODE(1, 0), i);

  QUIC_DVLOG_IF(1, true) << (i = 2);
  EXPECT_EQ(VALUE_BY_COMPILE_MODE(2, 0), i);
}

#undef VALUE_BY_COMPILE_MODE

TEST_F(QuicPlatformTest, QuicCHECK) {
  CHECK(1 == 1);
  CHECK(1 == 1) << " 1 == 1 is forever true.";

  EXPECT_DEBUG_DEATH({ DCHECK(false) << " Supposed to fail in debug mode."; },
                     "CHECK failed:.* Supposed to fail in debug mode.");
  EXPECT_DEBUG_DEATH({ DCHECK(false); }, "CHECK failed");

  EXPECT_DEATH_LOG_TO_STDERR({ CHECK(false) << " Supposed to fail in all modes."; },
                             "CHECK failed:.* Supposed to fail in all modes.");
  EXPECT_DEATH_LOG_TO_STDERR({ CHECK(false); }, "CHECK failed");
}

// Test the behaviors of the cross products of
//
//   {QUIC_LOG, QUIC_DLOG} x {FATAL, DFATAL} x {debug, release}
TEST_F(QuicPlatformTest, QuicFatalLog) {
#ifdef NDEBUG
  // Release build
  EXPECT_DEATH_LOG_TO_STDERR(QUIC_LOG(FATAL) << "Should abort 0", "Should abort 0");
  QUIC_LOG(DFATAL) << "Should not abort";
  QUIC_DLOG(FATAL) << "Should compile out";
  QUIC_DLOG(DFATAL) << "Should compile out";
#else
  // Debug build
  EXPECT_DEATH_LOG_TO_STDERR(QUIC_LOG(FATAL) << "Should abort 1", "Should abort 1");
  EXPECT_DEATH_LOG_TO_STDERR(QUIC_LOG(DFATAL) << "Should abort 2", "Should abort 2");
  EXPECT_DEATH_LOG_TO_STDERR(QUIC_DLOG(FATAL) << "Should abort 3", "Should abort 3");
  EXPECT_DEATH_LOG_TO_STDERR(QUIC_DLOG(DFATAL) << "Should abort 4", "Should abort 4");
#endif
}

TEST_F(QuicPlatformTest, QuicBranchPrediction) {
  GetLogger().set_level(INFO);

  if (QUIC_PREDICT_FALSE(rand() % RAND_MAX == 123456789)) {
    QUIC_LOG(INFO) << "Go buy some lottery tickets.";
  } else {
    QUIC_LOG(INFO) << "As predicted.";
  }
}

TEST_F(QuicPlatformTest, QuicNotReached) {
#ifdef NDEBUG
  QUIC_NOTREACHED(); // Expect no-op.
#else
  EXPECT_DEATH_LOG_TO_STDERR(QUIC_NOTREACHED(), "not reached");
#endif
}

TEST_F(QuicPlatformTest, QuicMutex) {
  QuicMutex mu;

  QuicWriterMutexLock wmu(&mu);
  mu.AssertReaderHeld();
  mu.WriterUnlock();
  {
    QuicReaderMutexLock rmu(&mu);
    mu.AssertReaderHeld();
  }
  mu.WriterLock();
}

TEST_F(QuicPlatformTest, QuicNotification) {
  QuicNotification notification;
  EXPECT_FALSE(notification.HasBeenNotified());
  notification.Notify();
  notification.WaitForNotification();
  EXPECT_TRUE(notification.HasBeenNotified());
}

TEST_F(QuicPlatformTest, QuicCertUtils) {
  bssl::UniquePtr<X509> x509_cert =
      Envoy::Extensions::TransportSockets::Tls::readCertFromFile(Envoy::TestEnvironment::substitute(
          "{{ test_rundir }}/test/extensions/transport_sockets/tls/test_data/san_dns_cert.pem"));
  // Encode X509 cert with DER encoding.
  unsigned char* der = nullptr;
  int len = i2d_X509(x509_cert.get(), &der);
  ASSERT_GT(len, 0);
  QuicStringPiece out;
  QuicCertUtils::ExtractSubjectNameFromDERCert(
      QuicStringPiece(reinterpret_cast<const char*>(der), len), &out);
  EXPECT_EQ("0z1\v0\t\x6\x3U\x4\x6\x13\x2US1\x13"
            "0\x11\x6\x3U\x4\b\f\nCalifornia1\x16"
            "0\x14\x6\x3U\x4\a\f\rSan Francisco1\r"
            "0\v\x6\x3U\x4\n\f\x4Lyft1\x19"
            "0\x17\x6\x3U\x4\v\f\x10Lyft Engineering1\x14"
            "0\x12\x6\x3U\x4\x3\f\vTest Server",
            out);
  OPENSSL_free(static_cast<void*>(der));
}

TEST_F(QuicPlatformTest, QuicTestOutput) {
  Envoy::TestEnvironment::setEnvVar("QUIC_TEST_OUTPUT_DIR", "/tmp", /*overwrite=*/false);

  // Set log level to INFO to see the test output path in log.
  GetLogger().set_level(INFO);

  EXPECT_LOG_NOT_CONTAINS("warn", "",
                          QuicRecordTestOutput("quic_test_output.1", "output 1 content\n"));
  EXPECT_LOG_NOT_CONTAINS("error", "",
                          QuicRecordTestOutput("quic_test_output.2", "output 2 content\n"));
  EXPECT_LOG_CONTAINS("info", "Recorded test output into",
                      QuicRecordTestOutput("quic_test_output.3", "output 3 content\n"));
}

class FileUtilsTest : public testing::Test {
public:
  FileUtilsTest() : dir_path_(Envoy::TestEnvironment::temporaryPath("quic_file_util_test")) {
    files_to_remove_.push(dir_path_);
  }

protected:
  void SetUp() override { Envoy::TestUtility::createDirectory(dir_path_); }

  void TearDown() override {
    while (!files_to_remove_.empty()) {
      const std::string& f = files_to_remove_.top();
      Envoy::TestEnvironment::removePath(f);
      files_to_remove_.pop();
    }
  }

  void addSubDirs(std::list<std::string> sub_dirs) {
    for (const std::string& dir_name : sub_dirs) {
      const std::string full_path = dir_path_ + "/" + dir_name;
      Envoy::TestUtility::createDirectory(full_path);
      files_to_remove_.push(full_path);
    }
  }

  void addFiles(std::list<std::string> files) {
    for (const std::string& file_name : files) {
      const std::string full_path = dir_path_ + "/" + file_name;
      { const std::ofstream file(full_path); }
      files_to_remove_.push(full_path);
    }
  }

  const std::string dir_path_;
  std::stack<std::string> files_to_remove_;
};

TEST_F(FileUtilsTest, ReadDirContents) {
  addSubDirs({"sub_dir1", "sub_dir2", "sub_dir1/sub_dir1_1"});
  addFiles({"file", "sub_dir1/sub_file1", "sub_dir1/sub_dir1_1/sub_file1_1", "sub_dir2/sub_file2"});

  EXPECT_THAT(ReadFileContents(dir_path_),
              testing::UnorderedElementsAre(dir_path_ + "/file", dir_path_ + "/sub_dir1/sub_file1",
                                            dir_path_ + "/sub_dir1/sub_dir1_1/sub_file1_1",
                                            dir_path_ + "/sub_dir2/sub_file2"));
}

TEST_F(FileUtilsTest, ReadFileContents) {
  const std::string data = "test string\ntest";
  const std::string file_path =
      Envoy::TestEnvironment::writeStringToFileForTest("test_envoy", data);
  std::string output;
  ReadFileContents(file_path, &output);
  EXPECT_EQ(data, output);
}

} // namespace
} // namespace quic
