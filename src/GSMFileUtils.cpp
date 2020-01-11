#include "Modem.h"

#include "GSMFileUtils.h"

GSMFileUtils::GSMFileUtils(bool debug):
    _count(0),
    _files(""),
    _fileTags{"USER", "FOAT", "PROFILE"}
{
    if (debug) {
        MODEM.debug();
    }
}

void GSMFileUtils::begin(unsigned long int timeout)
{
    int status;

    MODEM.begin();

    for (unsigned long start = millis(); (millis() - start) < timeout;) {
        status = _getFileList();
        if (status == 1) {
            _countFiles();
            return;
        }
        MODEM.poll();
    }

    Serial.println("Unable to read files.");
    while(true) {}
}

int GSMFileUtils::_getFileList()
{
    String response;

    MODEM.send("AT+ULSTFILE");
    int status = MODEM.waitForResponse(5000, &response);
    if (!response.length())
        return -1;

    if (status) {
        String list = response.substring(11);
        list.trim();
        _files = list;
    }

    return status;
}

void GSMFileUtils::_countFiles()
{
    String list = _files;
    size_t len = 0;

    if (list.length() > 0) {
        for(int index = list.indexOf(','); index != -1; index = list.indexOf(',')) {
            list.remove(0, index + 1);
            ++len;
        }
        ++len;
    }
    _count = len;
}

void GSMFileUtils::listFiles(String files[]) const
{
    String list = _files;
    int index;

    if (_count == 0)
        return;

    size_t n = 0;

    for(index = list.indexOf(','); index != -1; index = list.indexOf(',')) {
        String file = list.substring(1, index - 1);
        files[n++] = file;
        list.remove(0, index + 1);
    }
    files[n++] = list.substring(1, list.lastIndexOf("\""));
}

void GSMFileUtils::downloadFile(const String filename, const char buf[], size_t size, GSM_fileTags_t tag, const bool binary, const bool append)
{
    String response;
    bool fileExists = listFile(filename) > 0;

    if (fileExists && !append)
        deleteFile(filename);

    if (binary) {
        MODEM.send("ATE0");
        MODEM.waitForResponse();
    }

    MODEM.sendf("AT+UDWNFILE=\"%s\",%d,\"%s\"", filename.c_str(), size, _fileTags[tag]);
    MODEM.waitForPrompt(20000);

    for (size_t i = 0; i < size; i++)
        MODEM.write(buf[i]);

    int status = MODEM.waitForResponse(1000, &response);

    if (status && !append) {
        if (_count == 0)
            _files.concat("\"" + filename + "\"");
        else
            _files.concat(",\"" + filename + "\"");
        _count++;
    }

    if (binary) {
        MODEM.send("ATE1");
        MODEM.waitForResponse();
    }
}

void GSMFileUtils::fwUpdate() {
    // TODO: check if there is a FW with FOAT tag
    MODEM.sendf("AT+UFWINSTALL");
    MODEM.waitForPrompt(60000);
}

size_t GSMFileUtils::readFile(const String filename, String *content, const bool binary)
{
    String response;

    if (!listFile(filename)){
        Serial.println("File does not exist.");
        return 0;
    }

    if (binary)
        MODEM.binary();

    MODEM.sendf("AT+URDFILE=\"%s\"", filename.c_str());
    MODEM.waitForResponse(1000, &response);

    size_t skip = 10;
    String _content = response.substring(skip);
    int commaIndex = _content.indexOf(',');
    skip += commaIndex;

    _content = _content.substring(commaIndex + 1);
    commaIndex = _content.indexOf(',');
    skip += commaIndex;

    String sizePart = _content.substring(0, commaIndex);
    size_t size = sizePart.toInt();
    skip += 3;

    if (binary) {
        (*content).reserve(size);
        *content = "";
        for (size_t i = 0; i < size; i++) {
            *content += response[skip + i];
        }
    } else {
        _content = _content.substring(commaIndex + 2);
        *content = _content.substring(0, size);
    }

    if (binary)
        MODEM.noBinary();
    return size;
}

int GSMFileUtils::deleteFile(const String filename)
{
    String response;

    int start = _files.indexOf(filename);
    int count = filename.length();
    if (start == 1) {
        start = 0;
        count += 3;
    } else {
        start -= 2;
        count += 3;
    }

    MODEM.sendf("AT+UDELFILE=\"%s\"", filename.c_str());
    if (MODEM.waitForResponse(100, &response)){
        _files.remove(start, count);
        _count--;
        return 1;
    }
    return 0;
    }

int GSMFileUtils::deleteFiles()
{
    int n = 0;
    String files[_count];

    listFiles(files);

    while (_count > 0) {
        n += deleteFile(files[_count - 1]);
    }

    return n;
}

size_t GSMFileUtils::listFile(const String filename)
{
    String response;
    int res;
    size_t size = 0;

    MODEM.sendf("AT+ULSTFILE=2,\"%s\"", filename.c_str());
    res = MODEM.waitForResponse(100, &response);
    if (res == 1) {
        String content = response.substring(11);
        size = content.toInt();
    }

    return size;
}

size_t GSMFileUtils::freeSpace()
{
    String response;
    int res;
    size_t size = 0;

    MODEM.send("AT+ULSTFILE=1");
    res = MODEM.waitForResponse(100, &response);
    if (res == 1) {
        String content = response.substring(11);
        size = content.toInt();
    }

    return size;
}

void listFiles(const GSMFileUtils fu)
{
    // TODO: Print files sizes

    int count = fu.fileCount();
    String files[count];

    Serial.println(String(count) + " files found.");
    fu.listFiles(files);
    for (int i = 0; i < count; i++)
        Serial.println("File " + String(i + 1) + ": "  + files[i]);
}
