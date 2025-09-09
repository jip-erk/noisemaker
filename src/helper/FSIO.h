
#ifndef FSIO_h
#define FSIO_h

#include <Arduino.h>
#include <SD.h>
#include <string.h>

class FSIO {

    public:
        typedef struct {
            char sampleFilename[255] = "            ";
        } LibrarySample;

        FSIO();
        void listAllFiles();
        void listDirectory(File dir, int numTabs);
        
        void readLibrarySamplesFromSD(LibrarySample *librarySamples, String path);

        byte createSong(String filename);
        int getSongCount();
        int getSamplesCount(); // counts files in /SAMPLES directory -> global sample library
        boolean getSongName(int number, char *lineBuffer);
        boolean getSampleName(int number, char *lineBuffer);
        boolean getSampleFileName(int number, char *lineBuffer);
        String getSampleName(int number);
        String getSampleFileName(int number);

        void setSelectedSamplePathFromSD(int number);
        void setSelectedSamplePathFromSD(char *sampleName);
        void setSelectedSamplePathFromSD(String sampleName);
        char * getSelectedSamplePathFromSD();
        char * getSelectedSampleNameFromSD();

        LibrarySample * getLibrarySamples();

    private:
        char _selectedSamplePathFromSD[1024];
        char _selectedSampleNameFromSD[256];
        
        LibrarySample *_librarySamples;
        uint16_t _librarySamplesCount = 0;

        String _libraryPath = "/";        
};

#endif
