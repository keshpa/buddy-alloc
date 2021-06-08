#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

class Extent {
    public:
        Extent(size_t start, size_t length);

        pair<size_t, size_t> operator() () {
            return {start, length};
        }

        friend ostream& operator << (ostream& os, const Extent& ex);
    private:
        size_t start;
        size_t length;
};

ostream& operator << (ostream& os, const Extent& ex) {
    os << "Start: " << ex.start << ", End: " << ex.start+ex.length << " (Length: " << ex.length << ")" << endl;
    return os;
}

class RawBitBuddy {
    public:
        RawBitBuddy(size_t deviceSize, vector<Extent> doNotTrackList);

        Extent allocate(size_t size);

        void free(const Extent& ex);

        void setPolicy();

    private:
        vector<unsigned int> bitmask;
        size_t freeBlocks;
        size_t lastSavedId;
        size_t lastSavedCookie;
        mutable mutex mt;
};
