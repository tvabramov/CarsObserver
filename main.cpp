#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <sstream>
#include <thread>

#include "capturerfactory.h"
#include "crosscounterfactory.h"
#include "launchparams.h"
#include "recognizerfactory.h"
#include "trackerfactory.h"

using namespace std;
using namespace cv;
using namespace boost;
using namespace nlohmann;
namespace po = boost::program_options;

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

launchParams parseArgs(int argc, char *argv[]);
json getParamsFromJson(const string &_fn);

int main(int argc, char *argv[]) {
  cout << "Args list:" << endl;
  for (int i = 0; i < argc; ++i) cout << i << ": " << argv[i] << endl;

  logging::add_file_log(
      keywords::target =
          "logs",  // Needed for files rotation - deleting old files
      keywords::file_name = "sample_%N.log",
      keywords::rotation_size = 10 * 1024 * 1024,
      keywords::time_based_rotation =
          sinks::file::rotation_at_time_point(0, 0, 0),
      // keywords::format = "[%TimeStamp%]: %Severity%, %Message%"
      keywords::max_size = 100 * 1024 * 1024,
      keywords::scan_method = logging::sinks::file::scan_matching,
      keywords::format =
          (expr::stream << expr::format_date_time<boost::posix_time::ptime>(
                               "TimeStamp", "[%Y-%m-%d %H:%M:%S]")
                        << " <" << std::setw(7) << std::setfill(' ')
                        << logging::trivial::severity
                        << "> : " << expr::smessage));

  logging::add_common_attributes();

  BOOST_LOG_TRIVIAL(trace) << "CarsObserver has been started";
  // BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
  // BOOST_LOG_TRIVIAL(info) << "An informational severity message";
  // BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
  // BOOST_LOG_TRIVIAL(error) << "An error severity message";
  // BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

  launchParams lp = parseArgs(argc, argv);

  cout << "Launch parameters:" << endl;
  cout << "\tConfig file name = "
       << (lp.configFileName ? *lp.configFileName : "<undefined>") << endl;
  cout << endl;

  auto configJson =
      getParamsFromJson(lp.configFileName ? *lp.configFileName : "config.json");

  auto capturer = CapturerFactory::createCapturer(
      configJson.contains("Capturer") ? configJson["Capturer"] : json());

  auto recognizer = RecognizerFactory::createRecognizer(
      configJson.contains("Recognizer") ? configJson["Recognizer"] : json());

  auto tracker = TrackerFactory::createTracker(
      configJson.contains("Tracker") ? configJson["Tracker"] : json());

  auto cc = CrossCounterFactory::createCrossCounter(
      configJson.contains("CrossCounter") ? configJson["CrossCounter"]
                                          : json());

  bool abort = false;
  mutex stateMutex;

  // Tracker to analyzers thread
  bool t2othersStarted = false, t2othersFinished = false;
  std::thread t2othersThread([cc, tracker, &stateMutex, &abort,
                              &t2othersStarted, &t2othersFinished]() {
    {
      std::unique_lock<mutex> lck(stateMutex);
      t2othersStarted = true;
    }

    if (tracker)
      for (;;) {
        {
          std::unique_lock<mutex> lck(stateMutex);

          if (abort) break;
        }

        auto data = tracker->pop();

        if (cc) cc->push(std::move(data));

        BOOST_LOG_TRIVIAL(trace)
            << "Tracker-to-Others thread: data has been transferred";
      }

    BOOST_LOG_TRIVIAL(trace) << "Tracker-to-Others thread finished properly";

    {
      std::unique_lock<mutex> lck(stateMutex);
      t2othersFinished = true;
    }
  });
  t2othersThread.detach();

  // CrossCounter thread
  bool ccStarted = false, ccFinished = false;
  std::thread ccThread([cc, &stateMutex, &abort, &ccStarted, &ccFinished]() {
    {
      std::unique_lock<mutex> lck(stateMutex);
      ccStarted = true;
    }

    if (cc)
      for (;;) {
        {
          std::unique_lock<mutex> lck(stateMutex);

          if (abort) break;
        }

        cc->doWork();

        BOOST_LOG_TRIVIAL(trace) << "CrossCounter thread: doWork() done";
      }

    BOOST_LOG_TRIVIAL(trace) << "CrossCounter thread finished properly";

    {
      std::unique_lock<mutex> lck(stateMutex);
      ccFinished = true;
    }
  });
  ccThread.detach();

  // Recognizer thread
  bool recognizerStarted = false, recognizerFinished = false;
  std::thread recognizerThread([recognizer, &stateMutex, &abort,
                                &recognizerStarted, &recognizerFinished]() {
    {
      std::unique_lock<mutex> lck(stateMutex);
      recognizerStarted = true;
    }

    if (recognizer)
      for (;;) {
        {
          std::unique_lock<mutex> lck(stateMutex);

          if (abort) break;
        }

        recognizer->doWork();

        BOOST_LOG_TRIVIAL(trace) << "Recognizer thread: doWork() done";
      }

    BOOST_LOG_TRIVIAL(trace) << "Recognizer thread finished properly";

    {
      std::unique_lock<mutex> lck(stateMutex);
      recognizerFinished = true;
    }
  });
  recognizerThread.detach();

  // Tracker thread
  bool trackerStarted = false, trackerFinished = false;
  std::thread trackerThread(
      [tracker, &stateMutex, &abort, &trackerStarted, &trackerFinished]() {
        {
          std::unique_lock<mutex> lck(stateMutex);
          trackerStarted = true;
        }

        if (tracker)
          for (;;) {
            {
              std::unique_lock<mutex> lck(stateMutex);

              if (abort) break;
            }

            tracker->doWork();

            BOOST_LOG_TRIVIAL(trace) << "Tracker thread: doWork() done";
          }

        BOOST_LOG_TRIVIAL(trace) << "Tracker thread finished properly";

        {
          std::unique_lock<mutex> lck(stateMutex);
          trackerFinished = true;
        }
      });
  trackerThread.detach();

  // Video capturer thread
  bool capturerStarted = false, capturerFinished = false;
  std::thread capturerThread(
      [capturer, &stateMutex, &abort, &capturerStarted, &capturerFinished]() {
        {
          std::unique_lock<mutex> lck(stateMutex);
          capturerStarted = true;
        }

        if (capturer)
          for (;;) {
            {
              std::unique_lock<mutex> lck(stateMutex);

              if (abort) break;
            }

            capturer->doWork();

            BOOST_LOG_TRIVIAL(trace) << "Video capturer thread: doWork() done";
          }

        BOOST_LOG_TRIVIAL(trace) << "Video capturer thread finished properly";

        {
          std::unique_lock<mutex> lck(stateMutex);
          capturerFinished = true;
        }
      });
  // capturerThread.detach();

  // Capturer to Recognizer thread
  bool c2rStarted = false, c2rFinished = false;
  std::thread c2rThread(
      [capturer, recognizer, &stateMutex, &abort, &c2rStarted, &c2rFinished]() {
        {
          std::unique_lock<mutex> lck(stateMutex);
          c2rStarted = true;
        }

        if (capturer)
          for (;;) {
            {
              std::unique_lock<mutex> lck(stateMutex);

              if (abort) break;
            }

            auto data = capturer->pop();

            if (recognizer) recognizer->push(std::move(data));

            BOOST_LOG_TRIVIAL(trace)
                << "Capturer-to-Recognizer thread: data has been transferred";
          }

        BOOST_LOG_TRIVIAL(trace)
            << "Capturer-to-Recognizer thread finished properly";

        {
          std::unique_lock<mutex> lck(stateMutex);
          c2rFinished = true;
        }
      });
  c2rThread.detach();

  // Recognizer to others thread
  bool r2oStarted = false, r2oFinished = false;
  std::thread r2oThread([recognizer, tracker, &stateMutex, &abort, &r2oStarted,
                         &r2oFinished]() {
    {
      std::unique_lock<mutex> lck(stateMutex);
      r2oStarted = true;
    }

    if (recognizer)
      for (;;) {
        {
          std::unique_lock<mutex> lck(stateMutex);

          if (abort) break;
        }

        auto data = recognizer->pop();

        if (tracker) tracker->push(std::move(data));

        BOOST_LOG_TRIVIAL(trace)
            << "Recognizer-to-Others thread: data has been transferred";
      }

    BOOST_LOG_TRIVIAL(trace) << "Recognizer-to-Others thread finished properly";

    {
      std::unique_lock<mutex> lck(stateMutex);
      r2oFinished = true;
    }
  });
  r2oThread.detach();

  // Start working.
  std::cout << "Working started..." << endl;
  capturerThread.join();

  {
    std::unique_lock<mutex> lck(stateMutex);
    abort = true;
  }

  // Try three times to finish threads properly
  for (int i = 0; i < 3; ++i) {
    this_thread::sleep_for(std::chrono::milliseconds(200));

    std::unique_lock<mutex> lck(stateMutex);

    if (t2othersStarted && t2othersFinished && ccStarted && ccFinished &&
        recognizerStarted && recognizerFinished && trackerStarted &&
        trackerFinished && capturerStarted && capturerFinished && c2rStarted &&
        c2rFinished && r2oStarted && r2oFinished) {
      BOOST_LOG_TRIVIAL(trace) << "All treads has been finished properly";
      break;
    }
  }

  {
    std::unique_lock<mutex> lck(stateMutex);

    BOOST_LOG_TRIVIAL(trace) << "t2othersStarted = " << t2othersStarted;
    BOOST_LOG_TRIVIAL(trace) << "t2othersFinished = " << t2othersFinished;
    BOOST_LOG_TRIVIAL(trace) << "ccStarted = " << ccStarted;
    BOOST_LOG_TRIVIAL(trace) << "ccFinished = " << ccFinished;
    BOOST_LOG_TRIVIAL(trace) << "recognizerStarted = " << recognizerStarted;
    BOOST_LOG_TRIVIAL(trace) << "recognizerFinished = " << recognizerFinished;
    BOOST_LOG_TRIVIAL(trace) << "trackerStarted = " << trackerStarted;
    BOOST_LOG_TRIVIAL(trace) << "trackerFinished = " << trackerFinished;
    BOOST_LOG_TRIVIAL(trace) << "capturerStarted = " << capturerStarted;
    BOOST_LOG_TRIVIAL(trace) << "capturerFinished = " << capturerFinished;
    BOOST_LOG_TRIVIAL(trace) << "c2rStarted = " << c2rStarted;
    BOOST_LOG_TRIVIAL(trace) << "c2rFinished = " << c2rFinished;
    BOOST_LOG_TRIVIAL(trace) << "r2oStarted = " << r2oStarted;
    BOOST_LOG_TRIVIAL(trace) << "r2oFinished = " << r2oFinished;
  }

  return EXIT_SUCCESS;
}

json getParamsFromJson(const string &_fn) {
  json j;

  try {
    ifstream i(_fn);
    i >> j;
  } catch (const std::exception &e) {
    cerr << "Error while parsing file: " << e.what() << endl;
    BOOST_LOG_TRIVIAL(error) << "Error while parsing file: " << e.what();
  }

  return j;
}

launchParams parseArgs(int argc, char *argv[]) {
  // boost::program_options: https://habr.com/ru/post/174347/
  po::options_description desc("Command line options");
  desc.add_options()("help,h", "Show help")("config,c", po::value<string>(),
                                            "JSON config file name");

  po::variables_map vm;
  po::parsed_options parsed =
      po::command_line_parser(argc, argv).options(desc).run();
  po::store(parsed, vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << desc << endl;
    exit(EXIT_SUCCESS);
  }

  launchParams lp;

  if (vm.count("config")) lp.configFileName = vm["config"].as<string>();

  return lp;
}
