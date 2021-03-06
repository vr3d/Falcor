/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#include "Framework.h"
#include "Sample.h"
#include <algorithm>
#include <fstream>

namespace Falcor
{
    // Begin Test Frame.
    void SampleTest::beginTestFrame(SampleCallbacks* pSample)
    {
        // Check if we have any tests.
        if (!(mCurrentTimeTaskIndex < mTimeTasks.size() || mCurrentFrameTaskIndex < mFrameTasks.size()))
        {
            return;
        }

        mCurrentTriggerType = TriggerType::None;

        if (mCurrentTimeTaskIndex < mTimeTasks.size() && mCurrentTriggerType == TriggerType::None)
        {
            if (mTimeTasks[mCurrentTimeTaskIndex]->isActive(pSample))
            {
                mCurrentTriggerType = TriggerType::Time;

                mTimeTasks[mCurrentTimeTaskIndex]->onFrameBegin(pSample);
            }
        }

        if (mCurrentFrameTaskIndex < mFrameTasks.size() && mCurrentTriggerType == TriggerType::None)
        {
            if (mFrameTasks[mCurrentFrameTaskIndex]->isActive(pSample))
            {
                mCurrentTriggerType = TriggerType::Frame;

                mFrameTasks[mCurrentFrameTaskIndex]->onFrameBegin(pSample);
            }
        }
    }

    // End Test Frame.
    void SampleTest::endTestFrame(SampleCallbacks* pSample)
    {
        if (!(mCurrentTimeTaskIndex < mTimeTasks.size() || mCurrentFrameTaskIndex < mFrameTasks.size()))
        {
            return;
        }

        if (mCurrentTriggerType == TriggerType::Time)
        {
            mTimeTasks[mCurrentTimeTaskIndex]->onFrameEnd(pSample, this);

            if (mTimeTasks[mCurrentTimeTaskIndex]->mIsTaskComplete)
            {
                mCurrentTimeTaskIndex++;
            }
        }

        if (mCurrentTriggerType == TriggerType::Frame)
        {
            mFrameTasks[mCurrentFrameTaskIndex]->onFrameEnd(pSample, this);

            if (mFrameTasks[mCurrentFrameTaskIndex]->mIsTaskComplete)
            {
                mCurrentFrameTaskIndex++;
            }
        }
    }

    //Get the type of the next time task if there is one
    SampleTest::TaskType SampleTest::getNextTimeTaskType()
    {
        if (mTimeTasks.size() > mCurrentTimeTaskIndex)
        {
            return mTimeTasks[mCurrentTimeTaskIndex]->mTaskType;
        }
        else
        {
            return TaskType::None;
        }
    }

    //Get the type of the next frame task is there is one
    SampleTest::TaskType SampleTest::getNextFrameTaskType()
    {
        if (mFrameTasks.size() > mCurrentFrameTaskIndex)
        {
            return mFrameTasks[mCurrentFrameTaskIndex]->mTaskType;
        }
        else
        {
            return TaskType::None;
        }
    }

    // Write the JSON Literal.
    template<typename T>
    void SampleTest::writeJsonLiteral(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, const T& value)
    {
        rapidjson::Value jkey;
        jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);
        jval.AddMember(jkey, value, jallocator);
    }

    // Write the JSON Array.
    template<typename T>
    void SampleTest::writeJsonArray(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, const T& value)
    {
        rapidjson::Value jkey;
        jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);
        rapidjson::Value jvec(rapidjson::kArrayType);
        for (int32_t i = 0; i < value.length(); i++)
        {
            jvec.PushBack(value[i], jallocator);
        }

        jval.AddMember(jkey, jvec, jallocator);
    }

    // Write the JSON Value.
    void SampleTest::writeJsonValue(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, rapidjson::Value& value)
    {
        rapidjson::Value jkey;
        jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);
        jval.AddMember(jkey, value, jallocator);
    }

    // Write the JSON String.
    void SampleTest::writeJsonString(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, const std::string& value)
    {
        rapidjson::Value jstring, jkey;
        jstring.SetString(value.c_str(), (uint32_t)value.size(), jallocator);
        jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);

        jval.AddMember(jkey, jstring, jallocator);
    }

    // Write the JSON Bool.
    void SampleTest::writeJsonBool(rapidjson::Value& jval, rapidjson::Document::AllocatorType& jallocator, const std::string& key, bool isValue)
    {
        rapidjson::Value jbool, jkey;
        jbool.SetBool(isValue);
        jkey.SetString(key.c_str(), (uint32_t)key.size(), jallocator);

        jval.AddMember(jkey, jbool, jallocator);
    }

    // Write the Test Results.
    void SampleTest::writeJsonTestResults()
    {
        // Create the Test Results.
        rapidjson::Document jsonTestResults;
        jsonTestResults.SetObject();

        // Get the json Value and the Allocator.
        rapidjson::Value & jsonVal = jsonTestResults;
        auto & jsonAllocator = jsonTestResults.GetAllocator();

        writeJsonLiteral(jsonVal, jsonAllocator, "Frame Tasks", mFrameTasks.size());
        writeJsonLiteral(jsonVal, jsonAllocator, "Time Tasks", mTimeTasks.size());

        // Write the Json Test Results.
        writeJsonTestResults(jsonTestResults);

        // Get String Buffer for the json.
        rapidjson::StringBuffer jsonStringBuffer;

        // Get the PrettyWriter for the json.
        rapidjson::PrettyWriter<rapidjson::StringBuffer> jsonWriter(jsonStringBuffer);

        // Set the Indent.
        jsonWriter.SetIndent(' ', 4);

        // Use the jsonwriter.
        jsonTestResults.Accept(jsonWriter);

        // Construct the json string from the string buffer.
        std::string jsonString(jsonStringBuffer.GetString(), jsonStringBuffer.GetSize());

        std::string exeName = getExecutableName();
        std::string shortName = exeName.substr(0, exeName.size() - 4);

        std::string jsonFilename = "";

        if (mHasSetFilename)
        {
            jsonFilename = mTestOutputFilename + ".json";
        }
        else
        {
            // Write the json file.
            jsonFilename = shortName + ".json";
        }

        if (mHasSetDirectory)
        {
            jsonFilename = mTestOutputDirectory + jsonFilename;
        }

        std::ofstream outputStream(jsonFilename.c_str());
        if (outputStream.fail())
        {
            logError("Cannot write to " + jsonFilename + ".\n");
        }
        outputStream << jsonString;
        outputStream.close();
    }

    // Write the Json Test Results.
    void SampleTest::writeJsonTestResults(rapidjson::Document & jsonTestResults)
    {
        // Write the Load Time Check Results.
        writeLoadTimeCheckResults(jsonTestResults);

        // Write the Memory Range Results.
        writeMemoryRangesResults(jsonTestResults);

        // Write the Performance Range Results.
        writePerformanceRangesResults(jsonTestResults);

        // Write the Screen Capture Results.
        writeScreenCaptureResults(jsonTestResults);
    }

    // Write Load Time.
    void SampleTest::writeLoadTimeCheckResults(rapidjson::Document & jsonTestResults)
    {
        // Get the json Value and the Allocator.
        rapidjson::Value & jsonVal = jsonTestResults;
        auto & jsonAllocator = jsonTestResults.GetAllocator();

        if (mLoadTimeCheckTask != nullptr)
        {
            writeJsonLiteral(jsonVal, jsonAllocator, "Load Time Check", mLoadTimeCheckTask->mLoadTimeCheckResult);
        }
    }

    // Write the Memory Ranges Results.
    void SampleTest::writeMemoryRangesResults(rapidjson::Document & jsonTestResults)
    {
    }

    // Write the Performance Ranges Results.
    void SampleTest::writePerformanceRangesResults(rapidjson::Document & jsonTestResults)
    {
        auto & jsonAllocator = jsonTestResults.GetAllocator();

        // Write the screen captured image files to the output file.
        rapidjson::Value pcfArray(rapidjson::kArrayType);

        for (uint32_t i = 0; i < mFrameTasks.size(); i++)
        {
            if (mFrameTasks[i]->mTaskType == TaskType::PerformanceCheckTask)
            {
            }
        }

        jsonTestResults.AddMember("Performance Frame Checks", pcfArray, jsonAllocator);

        // Write the time based performance checks.
        rapidjson::Value pctArray(rapidjson::kArrayType);

        for (uint32_t i = 0; i < mTimeTasks.size(); i++)
        {
            if (mTimeTasks[i]->mTaskType == TaskType::PerformanceCheckTask)
            {
            }
        }

        jsonTestResults.AddMember("Performance Time Checks", pctArray, jsonAllocator);
    }

    // Write the Screen Capture Results.
    void SampleTest::writeScreenCaptureResults(rapidjson::Document & jsonTestResults)
    {
        auto & jsonAllocator = jsonTestResults.GetAllocator();

        // Write the screen captured image files to the output file.
        rapidjson::Value scfArray(rapidjson::kArrayType);

        for (uint32_t i = 0; i < mFrameTasks.size(); i++)
        {
            if (mFrameTasks[i]->mTaskType == TaskType::ScreenCaptureTask)
            {
                std::shared_ptr<ScreenCaptureFrameTask> scfTask = std::dynamic_pointer_cast<ScreenCaptureFrameTask>(mFrameTasks[i]);

                if (scfTask != nullptr)
                {
                    rapidjson::Value scffilename;
                    scffilename.SetString(scfTask->mCaptureFilename.c_str(), jsonAllocator);

                    rapidjson::Value scffilepath;
                    scffilepath.SetString(scfTask->mCaptureFilepath.c_str(), jsonAllocator);

                    rapidjson::Value scfFile;
                    scfFile.SetObject();
                    scfFile.AddMember("Filename", scffilename, jsonAllocator);
                    scfFile.AddMember("Filepath", scffilepath, jsonAllocator);

                    scfArray.PushBack(scfFile, jsonAllocator);
                }
            }
        }

        jsonTestResults.AddMember("Frame Screen Captures", scfArray, jsonAllocator);

        // Write the screen captured image files to the output file.
        rapidjson::Value sctArray(rapidjson::kArrayType);

        for (uint32_t i = 0; i < mTimeTasks.size(); i++)
        {
            if (mTimeTasks[i]->mTaskType == TaskType::ScreenCaptureTask)
            {
                std::shared_ptr<ScreenCaptureTimeTask> sctTask = std::dynamic_pointer_cast<ScreenCaptureTimeTask>(mTimeTasks[i]);

                if (sctTask != nullptr)
                {
                    rapidjson::Value sctfilename;
                    sctfilename.SetString(sctTask->mCaptureFilename.c_str(), jsonAllocator);

                    rapidjson::Value sctfilepath;
                    sctfilepath.SetString(sctTask->mCaptureFilepath.c_str(), jsonAllocator);

                    rapidjson::Value sctFile;
                    sctFile.SetObject();
                    sctFile.AddMember("Filename", sctfilename, jsonAllocator);
                    sctFile.AddMember("Filepath", sctfilepath, jsonAllocator);

                    sctArray.PushBack(sctFile, jsonAllocator);
                }
            }
        }

        jsonTestResults.AddMember("Time Screen Captures", sctArray, jsonAllocator);
    }

    // Initialize the Tests.
    void SampleTest::initializeTests(SampleCallbacks* pSample)
    {
        // Check for an Output Directory.
        auto argList = pSample->getArgList();
        if (argList.argExists("outputdir"))
        {
            std::vector<ArgList::Arg> odArgs = argList.getValues("outputdir");
            if (!odArgs.empty())
            {
                mHasSetDirectory = true;
                mTestOutputDirectory = odArgs[0].asString();
            }
        }

        // Check for a Results File.
        if (argList.argExists("outputfilename"))
        {
            std::vector<ArgList::Arg> orfArgs = argList.getValues("outputfilename");
            if (!orfArgs.empty())
            {
                mHasSetFilename = true;
                mTestOutputFilename = orfArgs[0].asString();
            }
        }

        if (argList.argExists("fixedtimedelta"))
        {
            std::vector<ArgList::Arg> ftdArgs = argList.getValues("fixedtimedelta");
            if (!ftdArgs.empty())
            {
                pSample->setFixedTimeDelta(ftdArgs[0].asFloat());
            }
        }

        // Ready the Frame Based Tests.
        initializeFrameTests(argList);

        // Ready the Time Based Tests.
        initializeTimeTests(argList);
    }

    // Initialize Frame Tests.
    void SampleTest::initializeFrameTests(const ArgList& args)
    {
        // Check for a Load Time.
        if (args.argExists("loadtime"))
        {
            mLoadTimeCheckTask = std::make_shared<LoadTimeCheckTask>();
            mFrameTasks.push_back(mLoadTimeCheckTask);
        }

        //
        // Check for a Shutdown Frame.
        if (args.argExists("shutdown"))
        {
            std::vector<ArgList::Arg> shutdownFrame = args.getValues("shutdown");
            if (!shutdownFrame.empty())
            {
                uint32_t startFrame = shutdownFrame[0].asUint();
                std::shared_ptr<ShutdownFrameTask> shutdownframeTask = std::make_shared<ShutdownFrameTask>(startFrame);
                mFrameTasks.push_back(shutdownframeTask);
            }
        }

        // Check for a Screenshot Frame.
        if (args.argExists("ssframes"))
        {
            std::vector<ArgList::Arg> ssFrames = args.getValues("ssframes");
            for (uint32_t i = 0; i < ssFrames.size(); ++i)
            {
                uint32_t captureFrame = ssFrames[i].asUint();
                std::shared_ptr<ScreenCaptureFrameTask> screenCaptureFrameTask = std::make_shared<ScreenCaptureFrameTask>(captureFrame);
                mFrameTasks.push_back(screenCaptureFrameTask);
            }
        }

        std::sort(mFrameTasks.begin(), mFrameTasks.end(), FrameTaskPtrCompare());
    }

    // Initialize Time Tests.
    void SampleTest::initializeTimeTests(const ArgList& args)
    {
        //
        // Check for a Shutdown Time.
        if (args.argExists("shutdowntime"))
        {
            //Shutdown
            std::vector<ArgList::Arg> shutdownTimeArg = args.getValues("shutdowntime");
            if (!shutdownTimeArg.empty())
            {
                float shutdownTime = shutdownTimeArg[0].asFloat();
                std::shared_ptr<ShutdownTimeTask> shutdowntimeTask = std::make_shared<ShutdownTimeTask>(shutdownTime);
                mTimeTasks.push_back(shutdowntimeTask);
            }
        }

        // Check for a Screenshot Frame.
        if (args.argExists("sstimes"))
        {
            std::vector<ArgList::Arg> ssTimes = args.getValues("sstimes");
            for (uint32_t i = 0; i < ssTimes.size(); ++i)
            {
                float captureTime = ssTimes[i].asFloat();
                std::shared_ptr<ScreenCaptureTimeTask> screenCaptureTimeTask = std::make_shared<ScreenCaptureTimeTask>(captureTime);
                mTimeTasks.push_back(screenCaptureTimeTask);
            }
        }

        // Check for Performance Time Ranges.
        if (args.argExists("perftimes"))
        {
            // Performance Check Frames.
            std::vector<ArgList::Arg> perfframeRanges = args.getValues("perftimes");

            if (perfframeRanges.size() % 2 != 0)
            {
                logError("Please provide a start and end frame for each Performance Frame Range. The extra one will be discarded.");
                perfframeRanges.pop_back();
            }

            for (uint32_t i = 0; i < perfframeRanges.size() / 2; i++)
            {
                std::shared_ptr<PerformanceCheckTimeTask> performanceCheckTimeTask = std::make_shared<PerformanceCheckTimeTask>(perfframeRanges[i].asFloat(), perfframeRanges[i + 1].asFloat());
            }
        }

        // Check for Memory Time Ranges.
        if (args.argExists("memtimes"))
        {
            // Memory Check Frames.
            std::vector<ArgList::Arg> memframeRanges = args.getValues("memtimes");
        }

        std::sort(mTimeTasks.begin(), mTimeTasks.end(), TimeTaskPtrCompare());
    }

    // Capture the Current Memory and write it to the provided memory check.
    void SampleTest::getMemoryStatistics(MemoryCheck & memoryCheck)
    {
#ifdef _WIN32 // Not supported on Linux
        memoryCheck.totalVirtualMemory = getTotalVirtualMemory();
        memoryCheck.totalUsedVirtualMemory = getUsedVirtualMemory();
        memoryCheck.currentlyUsedVirtualMemory = getProcessUsedVirtualMemory();
#endif
    }

    // Write the Memory Check Range, either in terms of Time or Frames to a file. Outputs Difference, Start and End Times and Memories.
    void SampleTest::writeMemoryRange(const MemoryCheckRange & memoryCheckRange, bool frameTest /*= true*/)
    {
        // Get the Strings for the Memory in Bytes - Start Frame
        std::string startTVM_B = std::to_string(memoryCheckRange.startCheck.totalVirtualMemory);
        std::string startTUVM_B = std::to_string(memoryCheckRange.startCheck.totalUsedVirtualMemory);
        std::string startCUVM_B = std::to_string(memoryCheckRange.startCheck.currentlyUsedVirtualMemory);

        std::string startTVM_MB = std::to_string(memoryCheckRange.startCheck.totalVirtualMemory / (1024 * 1024));
        std::string startTUVM_MB = std::to_string(memoryCheckRange.startCheck.totalUsedVirtualMemory / (1024 * 1024));
        std::string startCUVM_MB = std::to_string(memoryCheckRange.startCheck.currentlyUsedVirtualMemory / (1024 * 1024));

        // Check what the file description should say.
        std::string startCheck = "";
        if (frameTest)
        {
            startCheck = "At the Start Frame, " + std::to_string(memoryCheckRange.startCheck.frame) + ", : \n ";
        }
        else
        {
            startCheck = "At the Start Time, " + std::to_string(memoryCheckRange.startCheck.effectiveTime) + ", : \n";
        }
        startCheck = startCheck + ("Total Virtual Memory : " + startTVM_B + " bytes, " + startTVM_MB + " MB. \n");
        startCheck = startCheck + ("Total Used Virtual Memory By All Processes : " + startTUVM_B + " bytes, " + startTUVM_MB + " MB. \n");
        startCheck = startCheck + ("Virtual Memory used by this Process : " + startCUVM_B + " bytes, " + startCUVM_MB + " MB. \n \n");

        // Get the Strings for the Memory in Bytes - End Frame
        std::string endTVM_B = std::to_string(memoryCheckRange.endCheck.totalVirtualMemory);
        std::string endTUVM_B = std::to_string(memoryCheckRange.endCheck.totalUsedVirtualMemory);
        std::string endCUVM_B = std::to_string(memoryCheckRange.endCheck.currentlyUsedVirtualMemory);

        std::string endTVM_MB = std::to_string(memoryCheckRange.endCheck.totalVirtualMemory / (1024 * 1024));
        std::string endTUVM_MB = std::to_string(memoryCheckRange.endCheck.totalUsedVirtualMemory / (1024 * 1024));
        std::string endCUVM_MB = std::to_string(memoryCheckRange.endCheck.currentlyUsedVirtualMemory / (1024 * 1024));

        // Check what the file description should say.
        std::string endCheck = "";
        if (frameTest)
        {
            endCheck = "At the End Frame, " + std::to_string(memoryCheckRange.endCheck.frame) + ", : \n ";
        }
        else
        {
            endCheck = "At the End Time, " + std::to_string(memoryCheckRange.endCheck.effectiveTime) + ", : \n";
        }

        endCheck = endCheck + ("Total Virtual Memory : " + endTVM_B + " bytes, " + endTVM_MB + " MB. \n");
        endCheck = endCheck + ("Total Used Virtual Memory By All Processes : " + endTUVM_B + " bytes, " + endTUVM_MB + " MB. \n");
        endCheck = endCheck + ("Virtual Memory used by this Process : " + endCUVM_B + " bytes, " + endCUVM_MB + " MB. \n \n");

        // Compute the Difference Between the Two.
        std::string differenceCheck = "Difference : \n";
        int64_t difference = 0;
        {
            difference = (int64_t)memoryCheckRange.endCheck.currentlyUsedVirtualMemory - (int64_t)memoryCheckRange.startCheck.currentlyUsedVirtualMemory;
            differenceCheck = differenceCheck + std::to_string(difference) + "\n \n";
        }

        // Key string for difference.
        std::string keystring = "";
        if (frameTest)
        {
            keystring = std::to_string(memoryCheckRange.startCheck.frame) + " " + std::to_string(memoryCheckRange.endCheck.frame) + " " + (startCUVM_B)+" " + (endCUVM_B)+" " + std::to_string(difference) + " \n";
        }
        else
        {
            keystring = std::to_string(memoryCheckRange.startCheck.effectiveTime) + " " + std::to_string(memoryCheckRange.endCheck.effectiveTime) + " " + (startCUVM_B)+" " + (endCUVM_B)+" " + std::to_string(difference) + " \n";
        }

        // Get the name of the current program.
        std::string filename = getExecutableName();

        // Now we have a folder and a filename, look for an available filename (we don't overwrite existing files)
        std::string prefix = std::string(filename);
        // Frame Test.
        if (frameTest)
        {
            prefix = prefix + ".MemoryFrameCheck";
        }
        else
        {
            prefix = prefix + ".MemoryTimeCheck";
        }
        std::string executableDir = getExecutableDirectory();
        std::string txtFile;
        // Get an available filename.
        if (findAvailableFilename(prefix, executableDir, "txt", txtFile))
        {
            // Output the memory check.
            std::ofstream of;
            of.open(txtFile);
            of << keystring;
            of << differenceCheck;
            of << startCheck;
            of << endCheck;
            of.close();
        }
        else
        {
            // Log Error.
            logError("Could not find available filename when checking memory.");
        }
    }

    // Capture the Memory and return a representative string.
    void SampleTest::captureMemory(uint64_t frameCount, float currentTime, bool frameTest /*= true*/, bool endRange /*= false*/)
    {
        if (frameTest && !endRange && !mMemoryFrameCheckRange.active)
        {
            getMemoryStatistics(mMemoryFrameCheckRange.startCheck);
            mMemoryFrameCheckRange.startCheck.frame = frameCount;
            mMemoryFrameCheckRange.startCheck.effectiveTime = currentTime;
            mMemoryFrameCheckRange.active = true;
        }
        else if (frameTest && endRange && mMemoryFrameCheckRange.active)
        {
            getMemoryStatistics(mMemoryFrameCheckRange.endCheck);
            mMemoryFrameCheckRange.endCheck.frame = frameCount;
            mMemoryFrameCheckRange.endCheck.effectiveTime = currentTime;
            writeMemoryRange(mMemoryFrameCheckRange, true);
            mMemoryFrameCheckRange.active = false;
        }
        else if (!frameTest && !endRange && !mMemoryTimeCheckRange.active)
        {
            getMemoryStatistics(mMemoryTimeCheckRange.startCheck);
            mMemoryTimeCheckRange.startCheck.frame = frameCount;
            mMemoryTimeCheckRange.startCheck.effectiveTime = currentTime;
            mMemoryTimeCheckRange.active = true;
        }
        else if (!frameTest && endRange && mMemoryTimeCheckRange.active)
        {
            getMemoryStatistics(mMemoryTimeCheckRange.endCheck);
            mMemoryTimeCheckRange.endCheck.frame = frameCount;
            mMemoryTimeCheckRange.endCheck.effectiveTime = currentTime;
            writeMemoryRange(mMemoryTimeCheckRange, false);
            mMemoryTimeCheckRange.active = false;
        }
    }

    // LoadTimeCheckTask

    bool SampleTest::LoadTimeCheckTask::isActive(SampleCallbacks* pSample)
    {
        return pSample->getFrameID() == mStartFrame && !mIsTaskComplete;
    }

    void SampleTest::LoadTimeCheckTask::onFrameBegin(SampleCallbacks* pSample)
    {
        mLoadTimeCheckResult = pSample->getLastFrameTime();
    }

    void SampleTest::LoadTimeCheckTask::onFrameEnd(SampleCallbacks* pSample, SampleTest* pSampleTest)
    {
        // Task is Complete!
        mIsTaskComplete = true;
    }

    // ScreenCaptureFrameTask

    bool SampleTest::ScreenCaptureFrameTask::isActive(SampleCallbacks* pSample)
    {
        return pSample->getFrameID() == mCaptureFrame && !mIsTaskComplete;
    }

    void SampleTest::ScreenCaptureFrameTask::onFrameBegin(SampleCallbacks* pSample)
    {
        pSample->toggleText(false);
    }

    void SampleTest::ScreenCaptureFrameTask::onFrameEnd(SampleCallbacks* pSample, SampleTest* pSampleTest)
    {
        if (pSampleTest->mHasSetDirectory)
        {
            // Capture the Screen.
            std::string mCaptureFile = pSample->captureScreen(pSampleTest->mTestOutputFilename, pSampleTest->mTestOutputDirectory);
            mCaptureFilepath = getDirectoryFromFile(mCaptureFile);
            mCaptureFilename = getFilenameFromPath(mCaptureFile);
        }
        else
        {
            // Capture the Screen.
            std::string mCaptureFile = pSample->captureScreen(pSampleTest->mTestOutputFilename);
            mCaptureFilepath = getDirectoryFromFile(mCaptureFile);
            mCaptureFilename = getFilenameFromPath(mCaptureFile);
        }

        // Toggle the Text Back.
        pSample->toggleText(true);

        // Task is Complete!
        mIsTaskComplete = true;
    }

    // ShutdownFrameTask

    bool SampleTest::ShutdownFrameTask::isActive(SampleCallbacks* pSample)
    {
        return pSample->getFrameID() == mStartFrame && !mIsTaskComplete;
    }

    void SampleTest::ShutdownFrameTask::onFrameEnd(SampleCallbacks* pSample, SampleTest* pSampleTest)
    {
        // Write the json Test Results.
        pSampleTest->writeJsonTestResults();

        // Shutdown the App.
        pSample->shutdown();

        // On Test Shutdown.
        pSample->onTestShutdown();

        // Task is Complete!
        mIsTaskComplete = true;
    }

    // MemoryCheckTimeTask

    void SampleTest::MemoryCheckTimeTask::onFrameEnd(SampleCallbacks* pSample, SampleTest* pSampleTest)
    {
        auto currentTime = pSample->getCurrentTime();
        if (currentTime >= mStartTime && currentTime <= mEndTime && !mIsActive)
        {
            mIsActive = true;
            pSampleTest->getMemoryStatistics(mStartCheck);
        }

        if (currentTime >= mEndTime && mIsActive)
        {
            pSampleTest->getMemoryStatistics(mEndCheck);
            mIsActive = false;
            mIsTaskComplete = true;
        }
    }

    bool SampleTest::PerformanceCheckTimeTask::isActive(SampleCallbacks* pSample)
    {
        auto currentTime = pSample->getCurrentTime();
        return currentTime >= mStartTime && currentTime <= mEndTime;
    }

    void SampleTest::PerformanceCheckTimeTask::onFrameEnd(SampleCallbacks* pSample, SampleTest* pSampleTest)
    {
        // Task is Complete!
        mIsTaskComplete = true;
    }

    bool SampleTest::ScreenCaptureTimeTask::isActive(SampleCallbacks* pSample)
    {
        return mCaptureTime <= pSample->getCurrentTime() && !mIsTaskComplete;
    }

    void SampleTest::ScreenCaptureTimeTask::onFrameBegin(SampleCallbacks* pSample)
    {
        if (mCaptureTime <= pSample->getCurrentTime() && !mIsTaskComplete)
        {
            // Sneakily set the time of the program! For perfect pictures.
            pSample->setCurrentTime(mCaptureTime);

            pSample->toggleText(false);
        }
    }

    void SampleTest::ScreenCaptureTimeTask::onFrameEnd(SampleCallbacks* pSample, SampleTest* pSampleTest)
    {
        if (mCaptureTime <= pSample->getCurrentTime() && !mIsTaskComplete)
        {
            if (pSampleTest->mHasSetDirectory)
            {
                // Capture the Screen.
                std::string mCaptureFile = pSample->captureScreen(pSampleTest->mTestOutputFilename, pSampleTest->mTestOutputDirectory);
                mCaptureFilepath = getDirectoryFromFile(mCaptureFile);
                mCaptureFilename = getFilenameFromPath(mCaptureFile);
            }
            else
            {
                // Capture the Screen.
                std::string mCaptureFile = pSample->captureScreen(pSampleTest->mTestOutputFilename);
                mCaptureFilepath = getDirectoryFromFile(mCaptureFile);
                mCaptureFilename = getFilenameFromPath(mCaptureFile);
            }

            // Toggle the Text Back.
            pSample->toggleText(true);

            // Task is Complete!
            mIsTaskComplete = true;
        }
    }

    bool SampleTest::ShutdownTimeTask::isActive(SampleCallbacks* pSample)
    {
        return mShutdownTime <= pSample->getCurrentTime() && !mIsTaskComplete;
    }

    void SampleTest::ShutdownTimeTask::onFrameEnd(SampleCallbacks* pSample, SampleTest* pSampleTest)
    {
        if (mShutdownTime <= pSample->getCurrentTime() && !mIsTaskComplete)
        {
            // Write the json Test Results.
            pSampleTest->writeJsonTestResults();

            // Shutdown the App.
            pSample->shutdown();

            // On Test Shutdown.
            pSample->onTestShutdown();

            // Task is Complete!
            mIsTaskComplete = true;
        }
    }
}
