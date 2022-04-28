//----------------------------------------------------------------------
//Compile: Monitor.cpp
// Copyright (c) 2022, Guy Carver
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution.
//
//     * The name of Guy Carver may not be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// FILE    MonitorThread.ipp
//----------------------------------------------------------------------

constexpr uint32_t BUFFERSIZE = 0x10000;		//Size of input stream
constexpr DWORD TIMEOUT = 30;					//30 ms

using UniqueLock = std::unique_lock<std::mutex>;
using ResponseArray = std::vector<COMMAND>;
using ResponseQueue = std::queue<ResponsePtr>;	// Queue of pending responses to process
using CommandQueue = std::queue<CommandPtr>;	// Queue of pending commands to send

//----------------------------------------------------------------
///Class to handle communication with Vice binary monitor in a thread.
class Thread
{
public:
	//----------------------------------------------------------------
	Thread(  )
	{
		pInstance = std::unique_ptr<Thread>(this);
		MyThread = std::thread(&Thread::Runner, this);
	}

	//----------------------------------------------------------------
	~Thread(  )
	{
		Flush(true);							//Flush any pending commands
		StopRequest = true;						// Shut down the thread
		Close();
		MyThread.join();
	}

	//----------------------------------------------------------------
	static Thread &QInstance(  )
	{
		assert(pInstance);
		return *(pInstance.get());
	}

	//----------------------------------------------------------------
	static bool QInitialized(  ) { return pInstance != nullptr; }

	//----------------------------------------------------------------
	///Get connection status
	bool QConnected(  ) const { return Connected; }

	//----------------------------------------------------------------
	///Set not connected, called when we haven't received a response in a while
	void ClearConnected(  )
	{
		Close();								//Close socket
	}

	//----------------------------------------------------------------
	static void ShutDown(  )
	{
		pInstance = nullptr;
	}

	//----------------------------------------------------------------
	void PushCommand( CommandPtr apCommand )
	{
		//Don't do anything unless we are Connected
		if (Connected) {
			Diagnostics::AddCommand(*apCommand);

			{
				UniqueLock lock(QueueGuard);	// Lock for exclusive access
				CommandQ.push(apCommand);
			}
			NewCommands = true;					//Indicate we have new command to send
		}
	}

	//----------------------------------------------------------------
	void Flush( bool abWait = false )
	{
		if (Connected && NewCommands) {
			NewCommands = false;
			CommandsSema.release();			// Signal new data

			if (abWait) {
				while (!CommandQ.empty()) {
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}
			}
		}
	}

	//----------------------------------------------------------------
	///Call function on all responses in queue
	template<class FN>
	void ProcessResponses( FN aCallback )
	{
		if (ResponseTrigger) {
			UniqueLock lock(ResponseGuard);			// Lock for exclusive access

			//Iterate the map and report all Responses to the given callback function
			while (!Responses.empty()) {
				ResponsePtr pres = Responses.front();
				Responses.pop();
				aCallback(*pres);
			}
			ResponseTrigger = false;				//Clear the trigger
		}
	}

private:
	std::mutex QueueGuard;
	std::mutex ResponseGuard;
	std::counting_semaphore<100> CommandsSema{0};	// Semaphore for access to command queue
	CommandQueue CommandQ;						// Queue of pending commands to send
	ResponseQueue Responses;
	std::thread MyThread;
	SOCKET Sock = static_cast<SOCKET>(-1);
	bool Connected = false;
	bool StopRequest = false;
	bool NewCommands = false;
	bool ResponseTrigger = false;				// Trigger to indicate new response available
	uint8_t InBuffer[BUFFERSIZE];				// Buffer for receiving command responses from server

	static std::unique_ptr<Thread> pInstance;	// Singleton instance of this object

	//----------------------------------------------------------------
	///Main thread function
	void Runner(  )
	{
		//Loop until stop request received
		while (!StopRequest) {
			if (Connected) {
				//Attempt to acquire with a timeout then attempt to receive data
				while (CommandsSema.try_acquire_for(std::chrono::milliseconds(2))) {
					//Better not be empty if we got a signal!
					{
						UniqueLock lock(QueueGuard);	// Lock for exclusive access
						while (!CommandQ.empty()) {
							CommandPtr pcommand = CommandQ.front();
							CommandQ.pop();
							Send(pcommand);
						}
					}
				}

				//Attempt to receive responses
				if (auto count = Receive(); count) {
					ParseResponses(count);
				}
			}
			else {
				//Attempt to connect
				Connected = Open();
				if (!Connected) {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}
		}

		Connected = false;
	}

	//----------------------------------------------------------------
	///Send the given command to VICE
	bool Send( CommandPtr apCommand )
	{
		auto res = send(Sock, apCommand->AsBuffer(), apCommand->QSize(), 0);
		return res != SOCKET_ERROR;
	}

	//----------------------------------------------------------------
	///Receive commands from VICE into the buffer
	int32_t Receive(  )
	{
		int32_t total = 0;
		int32_t res = 0;
		//Loop until no data read or buffer is full
		while (total < BUFFERSIZE) {
			res = recv(Sock, reinterpret_cast<char*>(&InBuffer[total]), BUFFERSIZE - total, 0);
			if (res <= 0) break;					// If no data read, break out
			total += res;
		}

		return total;
	}

	//----------------------------------------------------------------
	///Parse all responses from the input buffer for size aCount
	void ParseResponses( uint32_t aCount )
	{
		auto pbuffer = InBuffer;
		//Loop til buffer consumed
		while (aCount)
		{
			ProcessRes res = Response::Process(pbuffer, aCount);
			if (res.pResponse) {
				Diagnostics::AddResponse(res.pResponse);
				{
					UniqueLock lock(ResponseGuard);				// Lock for exclusive access
					Responses.push(res.pResponse);
					ResponseTrigger = true;						// Set flag to tell main thread new data exists
				}
				pbuffer += res.Next;
				if (res.Next <= aCount) {
					aCount -= res.Next;
				}
				else {
					//We get here because the response was incomplete?
					aCount = 0;
				}
			}
			else {
				aCount = 0;							// No more good data
			}
		}
	}

	//----------------------------------------------------------------
	///Open Monitor
	/// returns true if successful
	bool Open(  )
	{
		//TODO: Non cout error handling so ImGui can print it
		WSADATA wsaData;
		bool bres = false;

		WORD dllVersion = MAKEWORD(2, 2);
		if (auto res = WSAStartup(dllVersion, &wsaData); !res) {
			if (Sock = socket(AF_INET, SOCK_STREAM, 0); Sock >= 0) {
				struct sockaddr_in server;
				server.sin_addr.s_addr = inet_addr(ViceIP);
				server.sin_family = AF_INET;
				server.sin_port = htons(VicePort);
				const DWORD to = TIMEOUT;
				//Set the receiver socket timeout
				setsockopt(Sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&to), sizeof(to));
				bres = connect(Sock , reinterpret_cast<sockaddr*>(&server) , sizeof(server)) >= 0;
			}
		}

		return bres;
	}

	//----------------------------------------------------------------
	///If socket is open, close it
	void Close(  )
	{
		Connected = false;

		//If socket open, close it
		if (Sock >= 0) {
			closesocket(Sock);
			WSACleanup();
			Sock = 0;
		}
	}
};

std::unique_ptr<Thread> Thread::pInstance;
