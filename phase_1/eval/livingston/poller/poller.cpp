#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <ctime>
#include <fstream>
#include <signal.h>

#include <sys/types.h>
#include <netdb.h>

#include "Messages.h"

#define PORT "8888"

bool cont = true;
bool songAdded = false;
bool trackAdded = false;

const char *memAllocIssue = "There were memory allocation issues.";
const char *msgParseIssue = "There were message parsing issues.";

void done(int signal)
{
    cont = false;
}

void generateRandomData(char **data, int *size)
{
    *data = (char *) ::malloc(*size);
    if (*data)
    {
        char *db = *data;
        for (int x = 0; x < *size; ++x)
        {
            *db = ::rand() % 0x100;
        }
    }
    else
    {
        *size = 0;
    }
}

void generateRandomString(char **data, int *size)
{
    *data = (char *) ::malloc(*size);
    if (*data)
    {
        char *db = *data;
        for (int x = 0; x < *size; ++x)
        {
            *db = (::rand() % 95) + 32;
        }
    }
    else
    {
        *size = 0;
    }
}

bool sendKnownMessage(ControlMessage **test, uint8_t reply, int sock, uint8_t *buffer, bool cond, uint8_t errCond, bool *updateMe)
{
    bool retVal = false;
    ControlMessage *m, *n;
    uint32_t amtRead = 0;

    m = *test;

    if (m)
    {
        send(sock, m, m->messageSize, 0);
        amtRead = read(sock ,buffer, 1024);
        n = parseMessage(buffer, amtRead);

        if (n)
        {
            if (n->messageType == reply && cond)
            {
                retVal = true;

                if (updateMe)
                {
                    *updateMe = true;
                }
            }
            else if (n->messageType == ERROR_RESPONSE && !cond)
            {
                ErrorResponse *er = (ErrorResponse *) (n + 1);
                if (er->errorCode == errCond)
                {
                    retVal = true;
                }
                else
                {
                    std::cerr << "Unexpected error code. (" << er->errorCode << " != " << errCond << ")" << std::endl;
                }
            }
            else
            {
                std::cerr << "Unexpected response to " << m->messageType << " is " << n->messageType << std::endl;
            }

            free(n);
            n = NULL;
        }
        else
        {
            std::cerr << msgParseIssue << std::endl;
        }

        free(*test);
        *test = NULL;
    }
    else
    {
        std::cerr << memAllocIssue << std::endl;
    }

    return retVal;
}

bool sendUnknownMessage(ControlMessage **test, int sock, uint8_t *buffer)
{
    bool retVal = false;
    ControlMessage *n, *m;
    uint32_t amtRead = 0;

    m = *test;

    if (m)
    {
        send(sock, m, m->messageSize, 0);
        amtRead = read(sock ,buffer, 1024);
        n = parseMessage(buffer, amtRead);

        if (n)
        {
            if (n->messageType == ERROR_RESPONSE)
            {
                ErrorResponse *er = (ErrorResponse *) (n + 1);
                if (er->errorCode == UNKNOWN_MESSAGE)
                {
                    retVal = true;
                }
                else
                {
                    std::cerr << "Unexpected error code (" << er->errorCode << " != " << UNKNOWN_MESSAGE << ")" <<std::endl;
                }
            }
            else
            {
                std::cerr << "Unexpected response to invalid message (" << m->messageType << ")" << std::endl;
            }

            free(n);
            n = NULL;
        }
        else
        {
            std::cerr << msgParseIssue << std::endl;
        }

        free(*test);
        *test = m = NULL;
    }
    else
    {
        std::cerr << memAllocIssue << std::endl;
    }

    return retVal;
}

bool doAction(int sock, int action)
{
    ControlMessage *m, *n;
    uint32_t amtRead = 0;
    uint8_t buffer[1024];
    bool retVal = false;

    switch(action)
    {
        case LOGIN_REQUEST:
            {
                uint64_t li = ::rand();
                m = makeLoginReqestMessage(li);
                if (m)
                {
                    send(sock, m, m->messageSize, 0);
                    amtRead = read(sock ,buffer, 1024);
                    n = parseMessage(buffer, amtRead);

                    if (n)
                    {
                        LoginReqestMessage *lir = (LoginReqestMessage *) (m + 1);

                        if ((lir->loginRequest & 0xFF) == LOGIN_ADMIN)
                        {
                            if (n->messageType == ERROR_RESPONSE)
                            {
                                ErrorResponse *er = (ErrorResponse *) (n + 1);
                                if (er->errorCode == INVALID_LOGIN)
                                {
                                    retVal = true;
                                }
                                else
                                {
                                     std::cerr << "Expected a different error code (" << er->errorCode << " != " << INVALID_LOGIN << ")" << std::endl;
                                }
                            }
                            else
                            {
                                std::cerr << "Unexpected response to invalid login. " << std::endl;
                            }
                        }
                        else
                        {
                            if (n->messageType == LOGIN_RESPONSE)
                            {
                                LoginResponseMessage *lirr = (LoginResponseMessage *) (n + 1);
                                if (lirr->loginResponse == LOGIN_NORMAL)
                                {
                                    retVal = true;
                                }
                                else
                                {
                                    std::cerr << "There was an issue with login validation (" << lirr->loginResponse << " != " << LOGIN_NORMAL << ")" << std::endl;
                                }
                            }
                            else
                            {
                                std::cerr << "There was an unexpected reply to login validation " << n->messageType << std::endl;
                            }
                        }

                        free(n);
                        n = NULL;
                    }
                    else
                    {
                        std::cerr << msgParseIssue << std::endl;
                    }

                    free(m);
                    m = NULL;
                }
                else
                {
                    std::cerr << memAllocIssue << std::endl;
                }
            }
            break;
        case LOGIN_RESPONSE:
            {
                uint64_t li = ::rand();
                m = makeLoginResponseMessage(li);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case PASSWORD_REQUEST:
            {
                char *psswd = NULL;
                bool stringPsswd = (bool) ::rand() % 2;
                int passwdLen = (::rand() % 1023) + 1;

                if (stringPsswd)
                {
                    generateRandomData(&psswd, &passwdLen);
                }
                else
                {
                    generateRandomString(&psswd, &passwdLen);
                }

                if (psswd)
                {
                    if (passwdLen > 0)
                    {
                        m = makePasswordValidationRequest(psswd);
                        retVal = sendKnownMessage(&m, ERROR_RESPONSE, sock, buffer, false, INVALID_PASSWORD, NULL);
                    }
                    else
                    {
                        std::cerr << "Memory and size are not in agreement" << std::endl;
                    }
                    free(psswd);
                    psswd = NULL;
                }
                else
                {
                    std::cerr << memAllocIssue << std::endl;
                }
            }
            break;
        case ADD_TRACK_REQUEST:
            {
                WaveFactory::WaveType wave;

                switch (::rand() % 5)
                {
                    case 0:
                        wave = WaveFactory::SINE;
                        break;
                    case 1:
                        wave = WaveFactory::SAW;
                        break;
                    case 2:
                        wave = WaveFactory::SQUARE;
                        break;
                    case 3:
                        wave = WaveFactory::TRIANGLE;
                        break;
                    case 4:
                        wave = WaveFactory::WHITE;
                        break;
                }

                uint32_t volume = ::rand();
                uint32_t lfo = ::rand();
                uint32_t depth = ::rand();
                uint32_t detune = ::rand();
                uint32_t rate = ::rand();

                m = makeAddTrackRequest(wave, volume, lfo, depth, detune, rate);
                retVal = sendKnownMessage(&m, ADD_TRACK_RESPONSE, sock, buffer, songAdded, INVALID_TRACK_ADD, &trackAdded);
            }
            break;
        case ADD_TRACK_RESPONSE:
            {
                uint32_t numTracks = ::rand();

                WaveFactory::WaveType wave;

                switch (::rand() % 5)
                {
                    case 0:
                        wave = WaveFactory::SINE;
                        break;
                    case 1:
                        wave = WaveFactory::SAW;
                        break;
                    case 2:
                        wave = WaveFactory::SQUARE;
                        break;
                    case 3:
                        wave = WaveFactory::TRIANGLE;
                        break;
                    case 4:
                        wave = WaveFactory::WHITE;
                        break;
                }

                uint32_t volume = ::rand();
                uint32_t lfo = ::rand();
                uint32_t depth = ::rand();
                uint32_t detune = ::rand();
                uint32_t rate = ::rand();

                m = makeAddTrackResponse(numTracks, wave, volume, lfo, depth, detune, rate);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case SET_TRACK_REQUEST:
            {
                uint64_t track = ::rand();
                m = makeSetTrackRequest(track);
                retVal = sendKnownMessage(&m, SET_TRACK_RESPONSE, sock, buffer, trackAdded, INVALID_TRACK_SET, NULL);
            }
            break;
        case SET_TRACK_RESPONSE:
            {
                uint64_t track = ::rand();
                m = makeSetTrackResponse(track);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case ADD_NOTE_REQUEST:
            {
                uint8_t tone = ::rand() % 0xff;
                int octave = ::rand();
                uint8_t beat = ::rand() % 0xff;

                m = makeAddNoteRequest((Semitone) tone, octave, (Tempo::Beat) beat);
                retVal = sendKnownMessage(&m, ADD_NOTE_RESPONSE, sock, buffer, trackAdded, INVALID_NOTE_ADD, NULL);
            }
            break;
        case ADD_NOTE_RESPONSE:
            {
                uint8_t tone = ::rand() % 0xff;
                int octave = ::rand();
                uint8_t beat = ::rand() % 0xff;

                m = makeAddNoteResponse((Semitone) tone, octave, (Tempo::Beat) beat);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case ADD_REST_REQUEST:
            {
                uint8_t b = ::rand() % 0xff;
                m = makeAddRestRequest((Tempo::Beat) b);
                retVal = sendKnownMessage(&m, ADD_REST_RESPONSE, sock, buffer, trackAdded, INVALID_NOTE_ADD, NULL);
            }
            break;
        case ADD_REST_RESPONSE:
            {
                uint8_t b = ::rand() % 0xff;
                m = makeAddRestResponse((Tempo::Beat) b);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case SET_TRACK_GAIN_REQUEST:
            {
                uint64_t gain = ::rand();
                m = makeSetTrackGainRequest(gain);
                retVal = sendKnownMessage(&m, SET_TRACK_GAIN_RESPONSE, sock, buffer, trackAdded, INVALID_TRACK_ACTION, NULL);
            }
            break;
        case SET_TRACK_GAIN_RESPONSE:
            {
                uint64_t gain = ::rand();
                m = makeSetTrackGainResponse(gain);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case ADD_DELAY_REQUEST:
            {
                uint32_t mix = ::rand();
                uint32_t delay = ::rand();
                uint8_t bDelay = ::rand() % 0xff;
                uint32_t feedback = ::rand();
                uint64_t prioroty = ::rand();
                m = makeAddDelayRequest(mix, delay, (Tempo::Beat) bDelay, feedback, prioroty);
                retVal = sendKnownMessage(&m, ADD_DELAY_RESPONSE, sock, buffer, trackAdded, INCALID_EFFECT_ADD, NULL);
            }
            break;
        case ADD_DELAY_RESPONSE:
            {
                uint32_t mix = ::rand();
                uint32_t delay = ::rand();
                uint8_t bDelay = ::rand() % 0xff;
                uint32_t feedback = ::rand();
                uint64_t prioroty = ::rand();
                m = makeAddDelayResponse(mix, delay, (Tempo::Beat) bDelay, feedback, prioroty);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case ADD_TREMOLO_REQUEST:
            {
                uint32_t mix = ::rand();
                uint32_t rate = ::rand();
                uint32_t mode = ::rand();
                uint32_t priority = rand();
                m = makeAddTremoloRequest(mix, rate, mode, priority);
                retVal = sendKnownMessage(&m, ADD_TREMOLO_RESPONSE, sock, buffer, trackAdded, INCALID_EFFECT_ADD, NULL);
            }
            break;
        case ADD_TREMOLO_RESPONSE:
            {
                uint32_t mix = ::rand();
                uint32_t rate = ::rand();
                uint32_t mode = ::rand();
                uint32_t priority = rand();
                m = makeAddTremoloResponse(mix, rate, mode, priority);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case ADD_CHORUS_REQUEST:
            {
                uint32_t mix = ::rand();
                uint32_t delay = ::rand();
                uint32_t depth = ::rand();
                uint32_t rate = ::rand();
                uint32_t priority = ::rand();
                m = makeAddChorusRequest(mix, delay, depth, rate, priority);
                retVal =sendKnownMessage(&m, ADD_CHORUS_RESPONSE, sock, buffer, trackAdded, INCALID_EFFECT_ADD, NULL);
            }
            break;
        case ADD_CHORUS_RESPONSE:
            {
                uint32_t mix = ::rand();
                uint32_t delay = ::rand();
                uint32_t depth = ::rand();
                uint32_t rate = ::rand();
                uint32_t priority = ::rand();
                m = makeAddChorusResponse(mix, delay, depth, rate, priority);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case ADD_EQ_BAND_REQUEST:
            {
                uint16_t frequency = ::rand() % 0xffff;
                uint16_t gain = ::rand() % 0xffff;
                uint32_t band = ::rand();
                uint64_t priority = ::rand();
                m = makeAddEqBandRequest(frequency, gain, band, priority);
                retVal = sendKnownMessage(&m, ADD_EQ_BAND_RESPONSE, sock, buffer, trackAdded, INCALID_EFFECT_ADD, NULL);
            }
            break;
        case ADD_EQ_BAND_RESPONSE:
            {
                uint16_t frequency = ::rand() % 0xffff;
                uint16_t gain = ::rand() % 0xffff;
                uint32_t band = ::rand();
                uint64_t priority = ::rand();
                m = makeAddEqBandResponse(frequency, gain, band, priority);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case REMOVE_EFFECT_REQUEST:
            {
                uint32_t priority = ::rand();
                m = makeRemoveEffectRequest(priority);

                if (priority <= 10)
                {
                    retVal = sendKnownMessage(&m, REMOVE_EFFECT_RESPONSE, sock, buffer, trackAdded, INCALID_EFFECT_ADD, NULL);
                }
                else
                {
                    retVal = sendKnownMessage(&m, REMOVE_EFFECT_RESPONSE, sock, buffer, false, INCALID_EFFECT_ADD, NULL);
                }

            }
            break;
        case REMOVE_EFFECT_RESPONSE:
            {
                uint32_t priority = ::rand();
                m = makeRemoveEffectResponse(priority);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case CREATE_SONG_REQUEST:
            {
                uint64_t tempo = ::rand();
                m = makeCreateSongRequest(tempo);
                retVal = sendKnownMessage(&m, CREATE_SONG_RESPONSE, sock, buffer, true, 0xFF, &songAdded);
            }
            break;
        case CREATE_SONG_RESPONSE:
            {
                uint64_t tempo = ::rand();
                m = makeCreateSongResponse(tempo);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case BOUNCE_SONG_REQUEST:
            {
                uint64_t clear = ::rand();
                m = makeBounceSongRequest(clear);
                send(sock, m, m->messageSize, 0);

                ControlMessage bounceControlData;
                amtRead = read(sock, &bounceControlData, sizeof(bounceControlData));

                BounceSongResponse *bounce = (BounceSongResponse *) malloc(bounceControlData.messageSize - sizeof(bounceControlData));
                int dataRead = 0, maxData = bounceControlData.messageSize - sizeof(bounceControlData);
                uint8_t *bounceData = (uint8_t *) bounce;

                do
                {
                    amtRead = read(sock, bounceData, 1024);
                    dataRead += amtRead;
                    bounceData += amtRead;
                } while (dataRead < maxData);

                free(bounce);

                retVal = true;
            }
            break;
        case BOUNCE_SONG_RESPONSE:
            {
                int size = (::rand() % 1023) + 1;
                char *b;
                generateRandomData(&b, &size);

                if (b)
                {
                    m = makeBounceSongResponse(size,(uint8_t *) b);
                    retVal = sendUnknownMessage(&m, sock, buffer);
                    free(b);
                    b = NULL;
                }
                else
                {
                    std::cerr << memAllocIssue << std::endl;
                }
            }
            break;
        case SHUTDOWN_REQUEST:
            {
                retVal = true;
            }
            break;
        case SHUTDOWN_RESPONSE:
            {
                m = makeShutdownResponse();
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case SPECIAL_KEY_REQUEST:
            {
                m = makeSpecialKeyRequest();
                retVal = sendKnownMessage(&m, ERROR_RESPONSE, sock, buffer, false, INVALID_REQUEST_FOR_FLAG, NULL);
            }
            break;
        case SPECIAL_KEY_RESPONSE:
            {
                char *name = NULL;
                int size = (::rand() % 1023) + 1;
                generateRandomString(&name, &size);

                if (name)
                {
                    m = makeSpecialKeyResponse(name);
                    retVal = sendUnknownMessage(&m, sock, buffer);
                    free(name);
                    name = NULL;
                }
                else
                {
                    std::cerr << memAllocIssue << std::endl;
                }
            }
            break;
        case SONG_INFO_REQUEST:
            {
                m = makeSongInfoRequest();
                retVal = sendKnownMessage(&m, SONG_INFO_RESPONSE, sock, buffer, songAdded, INVALID_SONG_ACTION, NULL);
            }
            break;
        case SONG_INFO_RESPONSE:
            {
                uint32_t numTracks = ::rand();
                uint32_t lenSamples = ::rand();
                uint32_t lenMillis = ::rand();
                int nameLen = (::rand() % 1023) + 1;
                char *name = NULL;

                generateRandomString(&name, &nameLen);

                if (name)
                {
                    m = makeSongInfoResponse(numTracks, lenSamples, lenMillis, name);
                    retVal = sendUnknownMessage(&m, sock, buffer);
                    free(name);
                    name = NULL;
                }
                else
                {
                    std::cerr << memAllocIssue << std::endl;
                }
            }
            break;
        case CLEAR_SONG_REQUEST:
            {
                m = makeClearSongRequest();
                bool clearStuff = false;
                retVal = sendKnownMessage(&m, CLEAR_SONG_RESPONSE, sock, buffer, true, 0xff, &clearStuff);
                if (clearStuff)
                {
                    songAdded = false;
                    trackAdded = false;
                }
            }
            break;
        case CLEAR_SONG_RESPONSE:
            {
                m = makeClearSongResponse();
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
        case SET_SONG_NAME_REQUEST:
            {
                char *name = NULL;
                int size = (::rand() % 1023) + 1;
                generateRandomString(&name, &size);

                if (name)
                {
                    m = makeSetNameRequest(name);
                    retVal = sendKnownMessage(&m, SET_SONG_NAME_RESPONSE, sock, buffer, songAdded, INVALID_SONG_ACTION, NULL);
                    free(name);
                    name = NULL;
                }
                else
                {
                    std::cerr << memAllocIssue << std::endl;
                }
            }
            break;
        case SET_SONG_NAME_RESPONSE:
            {
                char *name = NULL;
                int size = (::rand() % 1023) + 1;
                generateRandomString(&name, &size);

                if (name)
                {
                    m = makeSetNameResponse(name);
                    retVal = sendUnknownMessage(&m, sock, buffer);
                    free(name);
                    name = NULL;
                }
                else
                {
                    std::cerr << memAllocIssue << std::endl;
                }

            }
            break;
        case SAVE_STATE_REQUEST:
            {
                retVal = true;
            }
            break;
        case SAVE_STATE_RESPONSE:
            {
                int size = (::rand() % 1023) + 1;
                char *data = NULL;
                generateRandomData(&data, &size);

                if (data)
                {
                    m = makeSaveStateResponse(size,(uint8_t *) data);
                    retVal = sendUnknownMessage(&m, sock, buffer);
                    free(data);
                    data = NULL;
                }
                else
                {
                    std::cerr << memAllocIssue << std::endl;
                }

            }
            break;
        case ERROR_RESPONSE:
            {
                uint64_t error = ::rand();
                m = makeErrorResponse(error);
                retVal = sendUnknownMessage(&m, sock, buffer);
            }
            break;
    }

    if (!retVal)
    {
        std::cerr << "Failed test for action. (" << action << ")" << std::endl;
    }

    return retVal;
}

int main(int argc, char **argv)
{
    int seed, length;
    const char* envSeed = std::getenv("SEED");
    const char* envLength = std::getenv("LENGTH");
    const char* hostName = std::getenv("HOST");
    const char* portNum = std::getenv("PORT");
    const char* envTimeout = std::getenv("TIMEOUT");

    if (envSeed)
    {
        seed = ::atoi(envSeed);
        ::srand(seed);
    }
    else
    {
        ::srand(::time(NULL));
        seed = ::rand() + 1;
    }

    if (envLength)
    {
        length = ::atoi(envLength);
    }
    else
    {
        length = (::rand() % 6000) + 1000;
    }

    if (!hostName) {
      hostName = "127.0.0.1";
    }

    if (!portNum) {
      portNum = PORT;
    }

    int timeout = 1;
    if (envTimeout) {
      timeout = ::atoi(envTimeout);
    }

    std::cerr << "using seed: " << seed << std::endl;
    std::cerr << "using length: " << length << std::endl;

    struct addrinfo hints = {.ai_family = PF_UNSPEC,
                             .ai_socktype = SOCK_STREAM};
    struct addrinfo *res;
    struct addrinfo *res0;
    const char* cause = NULL;


    int addr_error = getaddrinfo(hostName, portNum, &hints, &res0);
    if (0 != addr_error) {
      std::cerr << "couldn't get address for " << hostName
                << ":\n"
                << gai_strerror(addr_error);
      return -1;
    }

        uint8_t buffer[1024] = {0};

        int sock = -1;

        for (res = res0; res; res = res->ai_next) {
          sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
          if (sock < 0)
            {
              cause = "couldn't make socket";
              continue;
            }

        if (connect(sock,
                    res->ai_addr, res->ai_addrlen) < 0)
        {
          cause = "couldn't connect to challenge";
          ::close(sock);
          sock = -1;
        }

        break;
        }

        if (sock < 0) {
          std::cerr << "failed: " << cause << std::endl;
          return -1;
        }

    int testsRan = 0;


    for (testsRan = 0; testsRan < length; testsRan++) {

      alarm(timeout);
        uint32_t action = ::rand() % (ERROR_RESPONSE + 1);
        bool success = doAction(sock, action);
        alarm(0);
        if (!success)
        {
            std::cerr << "Tests Ran: " << testsRan << std::endl;
            ControlMessage *c = makeShutdownRequest();
            std::cerr << "Failure: Shutting down the application" << std::endl;
            send(sock, c, c->messageSize, 0);
            std::cerr << "fail:(" << std::endl;
            return -1;
        }
    }

    ControlMessage *c = makeShutdownRequest();
    std::cerr << "Success: Shutting down the application" << std::endl;
    send(sock, c, c->messageSize, 0);

    close(sock);

    std::cerr << "Tests Ran: " << testsRan << std::endl;
    std::cerr << "success:)" << std::endl;
        return 0;
}
