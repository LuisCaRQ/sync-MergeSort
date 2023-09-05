#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>


using namespace std;
void printList(int vec[], int len);


class Semaphore {
public:
    Semaphore(int initialValue) : count_(initialValue) {}

    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (count_ <= 0) {
            condition_.wait(lock);
        }
        count_--;
    }

    void signal() {
        std::lock_guard<std::mutex> lock(mutex_);
        count_++;
        condition_.notify_one();
    }

private:
    int count_;
    std::mutex mutex_;
    std::condition_variable condition_;
};

class Barrier {
public:
    Barrier(int totalThreads) : totalThreads(totalThreads), currentThreads(0) {}

    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        currentThreads++;
        if (currentThreads < totalThreads) {
            condition.wait(lock);
        } else {
            currentThreads = 0;
            condition.notify_all();
        }
    }

private:
    int totalThreads;
    int currentThreads;
    std::mutex mutex;
    std::condition_variable condition;
};

class Monitor {
public:
    void enter() {
        std::unique_lock<std::mutex> lock(mutex);
        while (isOccupied) {
            condition.wait(lock);
        }
        isOccupied = true;
    }

    void exit() {
        std::lock_guard<std::mutex> lock(mutex);
        isOccupied = false;
        condition.notify_one();
    }

private:
    bool isOccupied = false;
    std::mutex mutex;
    std::condition_variable condition;
};


void merge(int vec[], int left, int middle, int right) {

	int lenVecLeft = middle - left + 1;
	int lenVecRight = right - middle;

	int vecL[lenVecLeft];
	int vecR[lenVecRight];


	for (int i = 0; i < lenVecLeft; i++)
        vecL[i] = vec[left + i];

    for (int i = 0; i < lenVecRight; i++)
        vecR[i] = vec[middle + 1 + i];

	int i = 0; 
    int j = 0; 
    int k = left; 

    while (i < lenVecLeft && j < lenVecRight) {
        if (vecL[i] <= vecR[j]) {
            vec[k] = vecL[i];
            i++;
        } else {
            vec[k] = vecR[j];
            j++;
        }
        k++;
    }

    while (i < lenVecLeft) {
        vec[k] = vecL[i];
        i++;
        k++;
    }
    while (j < lenVecRight) {
        vec[k] = vecR[j];
        j++;
        k++;
    }
}

void mergeSort(int* vec, int left, int right, Semaphore& semaphore) {
    if (left < right) {
        int middle = left + (right - left) / 2;

        Semaphore leftSemaphore(0);
        Semaphore rightSemaphore(0);

        std::thread leftThread(mergeSort, vec, left, middle, std::ref(leftSemaphore));
        std::thread rightThread(mergeSort, vec, middle + 1, right, std::ref(rightSemaphore));

        leftThread.join();
        rightThread.join();

        merge(vec, left, middle, right);

        semaphore.signal();
    } else {
        semaphore.signal(); 
    }
}

void printList(int vec[], int len) {
	for(int i = 0; i < len; i++){
		cout << vec[i];
		if(i < len-1)
			cout << ",";
		else
			cout << endl;
	}	
}

int main() {
    int numThreads = 4; 	
    Semaphore sem(0); 
    Semaphore mainSemaphore(1); 

    int len;
    cout << "Set list size:" << endl;
    cin >> len;
    int vec[len];

    for (int i = 0; i < len; i++) {
        cout << "Add: " << endl;
        cin >> vec[i];
    }
    cout << "Original list: ";
    printList(vec, len);

    int* ptrVec = vec;
    std::thread mergeSortThread(mergeSort, ptrVec, 0, len - 1, std::ref(sem));


    sem.wait();

    mergeSortThread.join();

    cout << "Ordered list: ";
    printList(vec, len);

    return 0;
}

