#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 512

// Function to convert decimal to binary char
char decToBinary(int n) {
    return static_cast<char>(n);
}

// ============================================================================
class fsInode {
    int fileSize;
    int block_in_use;

    int directBlock1;
    int directBlock2;
    int directBlock3;

    int singleInDirect;
    int doubleInDirect;
    int block_size;







    int doubleoffset;
    bool firsttime;

public:
    fsInode(int _block_size) {
        fileSize = 0;
        block_in_use = 0;
        block_size = _block_size;
        directBlock1 = -1;
        directBlock2 = -1;
        directBlock3 = -1;
        singleInDirect = -1;
        doubleInDirect = -1;



        doubleoffset = 0;
        firsttime = false;

    }

    ~fsInode() {
        // delete direct bloks
    }

    bool isFirsttime() const {
        return firsttime;
    }

    void setFirsttime(bool firsttime) {
        fsInode::firsttime = firsttime;
    }

    int getDoubleoffset() const {
        return doubleoffset;
    }

    void setDoubleoffset(int doubleoffset) {
        fsInode::doubleoffset = doubleoffset;
    }

    int getFileSize() {
        return fileSize;
    }

    int getBlocksInUse() {
        return block_in_use;
    }

    void setFileSize(int fileSize) {
        fsInode::fileSize = fileSize;
    }

    void setBlockInUse(int blockInUse) {
        block_in_use = blockInUse;
    }

    int getDirectBlock1() const {
        return directBlock1;
    }

    int getDirectBlock2() const {
        return directBlock2;
    }

    int getDirectBlock3() const {
        return directBlock3;
    }

    int getSingleInDirect() const {
        return singleInDirect;
    }

    int getDoubleInDirect() const {
        return doubleInDirect;
    }

    void setDirectBlock1(int directBlock1) {
        fsInode::directBlock1 = directBlock1;
    }

    void setDirectBlock2(int directBlock2) {
        fsInode::directBlock2 = directBlock2;
    }

    void setDirectBlock3(int directBlock3) {
        fsInode::directBlock3 = directBlock3;
    }

    void setSingleInDirect(int singleInDirect) {
        fsInode::singleInDirect = singleInDirect;
    }

    void setDoubleInDirect(int doubleInDirect) {
        fsInode::doubleInDirect = doubleInDirect;
    }


};

// ============================================================================
class FileDescriptor {
    pair<string, fsInode*> file;
    bool inUse;

public:
    FileDescriptor(string FileName, fsInode* fsi) {
        file.first = FileName;
        file.second = fsi;
        inUse = true;

    }

    // Add a member function to set the file name
    void setFileName(const string& newFileName) {
        file.first = newFileName;
    }

    string getFileName() {
        return file.first;
    }

    fsInode* getInode() {

        return file.second;

    }

    int GetFileSize() {
        return file.second->getFileSize();
    }
    bool isInUse() {
        return (inUse);
    }
    void setInUse(bool _inUse) {
        inUse = _inUse ;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk {
    FILE *sim_disk_fd;
    int block_size;
    bool is_formated;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize;//number of block
    int *BitVector;//array of size bit vector

    // Unix directories are lists of association structures,
    // each of which contains one filename and one inode number.
    map<string, fsInode*>  MainDir ;

    // OpenFileDescriptors --  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor

    vector< FileDescriptor > OpenFileDescriptors;// just for open




public:
    // ------------------------------------------------------------------------
    fsDisk() {
        sim_disk_fd = fopen( DISK_SIM_FILE , "w+" );
        assert(sim_disk_fd);
        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);

    }
    ~fsDisk() {

    }



    //The listAll function iterates through open file descriptors and displays their indices, names, usage status, and sizes.
    // It also prints the content of the simulated disk
    void listAll() {
        int i = 0;
        for ( auto it = begin (OpenFileDescriptors); it != end (OpenFileDescriptors); ++it) {
            cout << "index: " << i << ": FileName: " << it->getFileName() <<  " , isInUse: "
                 << it->isInUse() << " file Size: " << it->GetFileSize() << endl;
            i++;
        }
        char bufy;
        cout << "Disk content: '" ;
        for (i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
            cout << bufy;
        }
        cout << "'" << endl;


    }


    // The fsFormat function initializes the file system by formatting the disk with
    // specified block size and direct entry count
    void fsFormat(int blockSize = 4, int directEntries = 3) {

        if (blockSize == 1) {
            cout << "Block size of 1 is not supported. Please choose a different block size." << endl;
            return ;
        }

        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);

        block_size=blockSize;

        // Calculate the size of the BitVector.
        BitVectorSize = DISK_SIZE / blockSize;
        BitVector = new int[BitVectorSize];
        for (int i = 0; i < BitVectorSize; i++) {
            BitVector[i] = 0; // Initialize all blocks as free.
        }


// Clear the MainDir. && OpenFileDescriptors
        OpenFileDescriptors.clear();


        MainDir.clear();

        is_formated = true;

    }

    int CreateFile(string fileName) {
        if (!is_formated) {
            cout << "Cannot create a file on an unformatted disk." << endl;
        }

        if (MainDir.find(fileName) != MainDir.end()) {
            cout << "File '" << fileName << "' already exists." << endl;
        }

        // Create a new fsInode
        fsInode* newInode = new fsInode(block_size);

        // Add the file to the MainDir
        MainDir[fileName] = newInode;

        // Create a new FileDescriptor and add it to OpenFileDescriptors
        FileDescriptor newFileDescriptor(fileName, newInode);
        OpenFileDescriptors.push_back(newFileDescriptor);

        return OpenFileDescriptors.size() - 1;

    }

    // The OpenFile function is used to open an existing file in the file system
    int OpenFile(string FileName ) {
        if (!is_formated) {
            cout << "Cannot open a file on an unformatted disk." << endl;
            return -1;
        }

        if (MainDir.find(FileName) == MainDir.end()) {
            cout << "File not found: " << FileName << endl;
            return -1;
        }

        // Check if the file is already open
        for (int fd = 0; fd < OpenFileDescriptors.size(); ++fd) {
            if (OpenFileDescriptors[fd].isInUse() && OpenFileDescriptors[fd].getFileName() == FileName) {
                cout << "File is already open " << endl;
                return fd;
            }
        }

        for (int fd = 0; fd < OpenFileDescriptors.size(); ++fd) {
            if (!OpenFileDescriptors[fd].isInUse() && OpenFileDescriptors[fd].getFileName() == FileName) {
                OpenFileDescriptors[fd].setInUse(true);
                return fd;
            }
        }
        return OpenFileDescriptors.size() - 1;
    }

    //The CloseFile function is used to close a file associated with a given file descriptor (fd)
    string CloseFile(int fd) {

        if (fd < 0 || fd >= OpenFileDescriptors.size()) {
            cout << "Invalid file descriptor: " << fd << endl;
            return ""; // Return an error code or handle it according to your needs
        }

        string fileName = OpenFileDescriptors[fd].getFileName();
        OpenFileDescriptors[fd].setInUse(false);

        return fileName;

    }

    int AllocateFreeBlock(int *BitVector, int BitVectorSize) {
        // Iterate through the BitVector to find a free block
        for (int i = 0; i < BitVectorSize; ++i) {
            if (BitVector[i] == 0) {
                // Mark the block as used by setting it to 1
                BitVector[i] = 1;
                return i; // Return the block number (index)
            }
        }

        return -1;
    }


    // ------------------------------------------------------------------------

    // Helper function to get the current block number for writing
    int GetCurrentBlockNumber(fsInode* fileInode, int blockInUse) {
        if (blockInUse == 0) {
            return fileInode->getDirectBlock1();
        } else if (blockInUse == 1) {
            return fileInode->getDirectBlock2();
        } else if (blockInUse == 2) {
            return fileInode->getDirectBlock3();
        }
        return -1;
    }

// Helper function to allocate a new block and update the inode
    int AllocateAndSetBlock(fsInode* fileInode, int blockInUse, int* BitVector, int BitVectorSize) {
        int blockNumber = AllocateFreeBlock(BitVector, BitVectorSize);
        if (blockNumber == -1) {
            cout << "Disk is full. Cannot allocate more blocks." << endl;
            return -1;
        }

        // Update the direct block in the inode
        if (blockInUse == 0) {
            fileInode->setDirectBlock1(blockNumber);
        } else if (blockInUse == 1) {
            fileInode->setDirectBlock2(blockNumber);
        } else if (blockInUse == 2) {
            fileInode->setDirectBlock3(blockNumber);
        }
        return blockNumber;
    }



    int WriteToFile(int fd, char *buf, int len ) {

        if (!is_formated) {
            cout << "Cannot open a file on an unformatted disk." << endl;
            return -1;
        }

        if (fd < 0 || fd >= OpenFileDescriptors.size()) {
            cout << "Invalid file descriptor: " << fd << endl;
            return -1;
        }

        if (!OpenFileDescriptors[fd].isInUse()) {
            cout << "File is not open." << endl;
            return -1;
        }


        fsInode* fileInode = OpenFileDescriptors[fd].getInode();
        int blockSize = block_size;
        int fileSize = fileInode->getFileSize();
        int blockInUse = fileInode->getBlocksInUse();
        int bytesWritten = 0;


        while (bytesWritten < len) {
            if (blockInUse < 3) {
                // Calculate the current offset within the block
                int offset = fileSize % blockSize;
                int remainingSpaceInBlock = blockSize - offset;
                int bytesToWrite = min(remainingSpaceInBlock, len - bytesWritten);

                // Calculate the current block number
                int blockNumber = GetCurrentBlockNumber(fileInode, blockInUse);

                // Allocate a new block if we need
                if (blockNumber == -1 || offset == 0) {
                    blockNumber = AllocateAndSetBlock(fileInode, blockInUse, BitVector, BitVectorSize);
                    if (blockNumber == -1) {
                        break;
                    }
                }

                int fileOffset = blockNumber * blockSize + offset;
                int ret_val = fseek(sim_disk_fd, fileOffset, SEEK_SET);

                ret_val = fwrite(buf + bytesWritten, 1, bytesToWrite, sim_disk_fd);

                fileSize += bytesToWrite;
                bytesWritten += bytesToWrite;

                // If the current block is full, set it to -1 to allocate a new one next time
                if (offset + bytesToWrite == blockSize) {
                    blockNumber = -1;
                    blockInUse++;
                }
            } else if (blockInUse < 3 + block_size) {
                int single_offset = (fileSize - (3 * blockSize)) / blockSize;
                int sub_single_offset = (fileSize - (3 * blockSize)) % blockSize;
                int blockNumber = fileInode->getSingleInDirect();
                int remainingSpaceInBlock = blockSize - sub_single_offset;
                int bytesToWriteInSubBlock = min(remainingSpaceInBlock, len - bytesWritten);
                // Allocate a new sub-block within the single indirect block if necessary
                int subBlockNumber = -1;
                char charNum;
                bool firsttime = false;
                int get_single =fileInode->getSingleInDirect();
                if (get_single == -1) {
                    blockNumber = AllocateFreeBlock(BitVector, BitVectorSize);

                    fileInode->setSingleInDirect(blockNumber);
                    subBlockNumber = AllocateFreeBlock(BitVector, BitVectorSize);
                    charNum = decToBinary(subBlockNumber);
                    fseek(sim_disk_fd,  blockNumber* blockSize, SEEK_SET);
                    fwrite(&charNum, 1, 1, sim_disk_fd);
                    blockInUse++;
                    firsttime = true;


                }
               else if(remainingSpaceInBlock==blockSize && firsttime == false){
                    subBlockNumber = AllocateFreeBlock(BitVector,BitVectorSize);
                    charNum = decToBinary(subBlockNumber);
                    fseek(sim_disk_fd,blockNumber*blockSize + single_offset,SEEK_SET);
                    fwrite(&charNum,1,1,sim_disk_fd);
                    blockInUse++;
                }
                else{
                    fseek(sim_disk_fd, (blockNumber * blockSize) + single_offset, SEEK_SET);
                    fread(&charNum, 1, 1, sim_disk_fd);
                    subBlockNumber = static_cast<int>(charNum);
                }
                fseek(sim_disk_fd, (subBlockNumber * blockSize) + sub_single_offset, SEEK_SET);
                fwrite(&buf[bytesWritten], 1, bytesToWriteInSubBlock, sim_disk_fd);


                bytesWritten += bytesToWriteInSubBlock;


                fileSize += bytesToWriteInSubBlock;


            }
        else if (blockInUse < (3 + block_size + (block_size*blockSize))) {
                int double_offset = fileInode->getDoubleoffset();
                bool enter = false;
                int sub_single_offset = (fileSize - (3 + blockSize) * blockSize) / blockSize % blockSize;
                int sub_blocks_offset = (fileSize - (3 + blockSize) * blockSize) % blockSize;
                int blockNumber = fileInode->getDoubleInDirect();
                int remainingSpaceInBlock = blockSize - sub_blocks_offset;
                int bytesToWriteInSubBlock = min(remainingSpaceInBlock, len - bytesWritten);

                int subSingleNumber = -1;
                int subBlockNumber = -1;
                char charNum;
                int get_double = fileInode->getDoubleInDirect();


                if( fileInode->isFirsttime()  == true && sub_single_offset == 0   && sub_blocks_offset == 0) {
                    fileInode->setDoubleoffset(double_offset+1);
                    double_offset = fileInode->getDoubleoffset();
                    enter = true;}
                fseek(sim_disk_fd, (blockNumber * blockSize) + double_offset, SEEK_SET);
                fread(&charNum, 1, 1, sim_disk_fd);
                subSingleNumber = static_cast<int>(charNum);


                if (get_double == -1) {
                    blockNumber = AllocateFreeBlock(BitVector, BitVectorSize);
                    fileInode->setDoubleInDirect(blockNumber);

                    subSingleNumber = AllocateFreeBlock(BitVector, BitVectorSize);
                    charNum = decToBinary(subSingleNumber);
                    fseek(sim_disk_fd,  blockNumber* blockSize, SEEK_SET);
                    fwrite(&charNum, 1, 1, sim_disk_fd);

                    subBlockNumber = AllocateFreeBlock(BitVector, BitVectorSize);
                    charNum = decToBinary(subBlockNumber);
                    fseek(sim_disk_fd,  subSingleNumber* blockSize, SEEK_SET);
                    fwrite(&charNum, 1, 1, sim_disk_fd);
                    blockInUse++;

                    fileInode->setFirsttime(true);
                }
                 else if(enter == true || (remainingSpaceInBlock==blockSize && fileInode->isFirsttime() == true)){
                    if(enter != false){
                        enter = false ;
                        fseek(sim_disk_fd,  blockNumber* blockSize  + double_offset,  SEEK_SET);
                        subSingleNumber = AllocateFreeBlock(BitVector, BitVectorSize);
                        charNum = decToBinary(subSingleNumber);
                        fwrite(&charNum, 1, 1, sim_disk_fd);
                    }
                    fseek(sim_disk_fd,subSingleNumber*blockSize + sub_single_offset,SEEK_SET);
                    subBlockNumber = AllocateFreeBlock(BitVector,BitVectorSize);
                    charNum = decToBinary(subBlockNumber);
                    fwrite(&charNum,1,1,sim_disk_fd);
                    blockInUse++;
                 }
                else{
                    fseek(sim_disk_fd, (subSingleNumber * blockSize) + sub_single_offset, SEEK_SET);
                    fread(&charNum, 1, 1, sim_disk_fd);
                    subBlockNumber = static_cast<int>(charNum);
                }
                fseek(sim_disk_fd, (subBlockNumber * blockSize) + sub_blocks_offset, SEEK_SET);
                fwrite(&buf[bytesWritten], 1, bytesToWriteInSubBlock, sim_disk_fd);
                bytesWritten += bytesToWriteInSubBlock;
                fileSize += bytesToWriteInSubBlock;


            }
        else{
                break;
        }
        }

        // Update file size and blocks in use in the inode
        fileInode->setFileSize(fileSize);
        fileInode->setBlockInUse(blockInUse);
        return bytesWritten;
    }
    // ------------------------------------------------------------------------
    int DelFile( string FileName ) {

        if (!is_formated) {
            cout << "Cannot delete a file on an unformatted disk." << endl;
            return -1;
        }

        if (MainDir.find(FileName) == MainDir.end()) {
            cout << "File not found: " << FileName << endl;
            return -1;
        }



        fsInode* fileInode = MainDir[FileName];

        int blockSize = block_size;

        // Delete the direct blocks
        int directBlock1 = fileInode->getDirectBlock1();
        int directBlock2 = fileInode->getDirectBlock2();
        int directBlock3 = fileInode->getDirectBlock3();

        if (directBlock1 != -1) {
            BitVector[directBlock1] = 0;
            fileInode->setDirectBlock1(-1);
        }

        if (directBlock2 != -1) {
            BitVector[directBlock2] = 0;
            fileInode->setDirectBlock2(-1);
        }

        if (directBlock3 != -1) {
            BitVector[directBlock3] = 0;
            fileInode->setDirectBlock3(-1);
        }
        fileInode->setFileSize(0);
        fileInode->setBlockInUse(0);


        // Delete the single indirect block and its sub-blocks
        if (fileInode->getSingleInDirect() != -1) {
            int singleBlockNumber = fileInode->getSingleInDirect();
            BitVector[singleBlockNumber] = 0;
            fileInode->setSingleInDirect(-1);

            fileInode->setFileSize(0);
            fileInode->setBlockInUse(0);

            // Iterate through the sub-blocks and mark them as free
            for (int i = 0; i < blockSize; ++i) {
                int subBlockNumber;
                fseek(sim_disk_fd, singleBlockNumber * blockSize + i * sizeof(char), SEEK_SET);
                char charNum;
                fread(&charNum, 1, 1, sim_disk_fd);
                subBlockNumber = static_cast<int>(charNum);

                if (subBlockNumber != -1) {
                    BitVector[subBlockNumber] = 0;
                }
            }
        }


 //Delete the double indirect block and its sub-blocks
        if (fileInode->getDoubleInDirect() != -1) {
            int doubleBlockNumber = fileInode->getDoubleInDirect();
            BitVector[doubleBlockNumber] = 0;
            fileInode->setDoubleInDirect(-1);

            fileInode->setFileSize(0);
            fileInode->setBlockInUse(0);

            for (int i = 0; i < blockSize; ++i) {
                int singleBlockNumber;
                fseek(sim_disk_fd, doubleBlockNumber * blockSize + i * sizeof(char), SEEK_SET);
                char charNum;
                fread(&charNum, 1, 1, sim_disk_fd);
                singleBlockNumber = static_cast<int>(charNum);

                if (singleBlockNumber != -1) {
                    BitVector[singleBlockNumber] = 0;

                    // iterate through the sub-blocks of the single indirect block
                    for (int j = 0; j < blockSize; ++j) {
                        int subBlockNumber;
                        fseek(sim_disk_fd, singleBlockNumber * blockSize + j * sizeof(char), SEEK_SET);
                        fread(&charNum, 1, 1, sim_disk_fd);
                        subBlockNumber = static_cast<int>(charNum);

                        if (subBlockNumber != -1) {
                            BitVector[subBlockNumber] = 0;
                        }
                    }
                }
            }
        }

        // Remove the file's entry from MainDir
        MainDir.erase(FileName);

        // Find the associated file descriptor and mark it as not in use
        int file_descriptor;
        for (file_descriptor = 0; file_descriptor < OpenFileDescriptors.size(); file_descriptor++) {
            if ( OpenFileDescriptors[file_descriptor].getFileName() == FileName) {
                OpenFileDescriptors[file_descriptor].setInUse(false);
                OpenFileDescriptors[file_descriptor].setFileName("");
                break;
            }
        }

        return file_descriptor;


    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len ) {

        if (!is_formated) {
            cout << "Cannot open a file on an unformatted disk." << endl;
            return -1;
        }

        // Check if the file descriptor is valid
        if (fd < 0 || fd >= OpenFileDescriptors.size()) {
            cout << "Invalid file descriptor: " << fd << endl;
            return -1;
        }

        // Check if the file is open
        if (!OpenFileDescriptors[fd].isInUse()) {
            cout << "File is not open." << endl;
            return -1;
        }

        // Get the file name associated with the file descriptor
        string fileName = OpenFileDescriptors[fd].getFileName();


        // Check if the file exists in the MainDir
        if (MainDir.find(fileName) == MainDir.end()) {
            cout << "File not found: " << fileName << endl;
            return -1;
        }
        memset(buf,0,DISK_SIZE);
        fsInode* fileInode = OpenFileDescriptors[fd].getInode();
        int blockSize = block_size;
        int fileSize = fileInode->getFileSize();
        int bytesRead = 0;

        // Check if the file has been deleted
        if (fileSize == 0) {
            cout << "File has been deleted. Cannot read from it." << endl;
            return -1;
        }


        while (bytesRead < len) {
            if (bytesRead >= fileSize) {
                // We have read the entire file, no more data to read.
                break;
            }

            // Calculate the current offset within the file
            int offset = bytesRead % blockSize;
            int remainingBytesInBlock = blockSize - offset;
            int bytesToRead = min(remainingBytesInBlock, len - bytesRead);

            // Calculate the current block number
            int blockIndex = bytesRead / blockSize;
            int blockNumber = -1;

            // Read from direct blocks
            if (blockIndex < 3) {
                blockNumber = GetCurrentBlockNumber(fileInode, blockIndex);
                if (blockNumber == -1) {
                    break;
                }

                // Read from the single indirect block
            } else if (blockIndex < 3 + blockSize) {
                int singleBlockNumber = fileInode->getSingleInDirect();
                if (singleBlockNumber == -1) {
                    break;
                }



                // Read the block number from the single indirect block
                fseek(sim_disk_fd, singleBlockNumber * blockSize + (blockIndex - 3) * sizeof(char), SEEK_SET);
                char charNum;
                fread(&charNum, 1, 1, sim_disk_fd);
                blockNumber = static_cast<int>(charNum);
            } else if (blockIndex < 3 + blockSize + blockSize * blockSize){

                int doubleBlockNumber = fileInode->getDoubleInDirect();
                if (doubleBlockNumber == -1) {

                    break;
                }

                // Calculate the offset within the double indirect block
                int doubleOffset = (blockIndex - 3 - blockSize) / blockSize;
                int singleOffset = (blockIndex - 3 - blockSize) % blockSize;

                // Read the single indirect block number from the double indirect block
                fseek(sim_disk_fd, doubleBlockNumber * blockSize + doubleOffset * sizeof(char), SEEK_SET);
                char charNum;
                fread(&charNum, 1, 1, sim_disk_fd);
                int singleBlockNumber = static_cast<int>(charNum);

                // Read the block number from the single indirect block
                fseek(sim_disk_fd, singleBlockNumber * blockSize + singleOffset * sizeof(char), SEEK_SET);
                fread(&charNum, 1, 1, sim_disk_fd);
                blockNumber = static_cast<int>(charNum);
            }


            if (blockNumber == -1) {
                // No more blocks to read
                break;
            }

            // Seek to the correct position in the disk file
            int fileOffset = blockNumber * blockSize + offset;
            int ret_val = fseek(sim_disk_fd, fileOffset, SEEK_SET);

            // Read data from the disk
            ret_val = fread(buf + bytesRead, 1, bytesToRead, sim_disk_fd);

            bytesRead += ret_val;

            // If we've reached the end of the file, break the loop
            if (bytesRead == fileSize) {
                break;
            }
        }

        return bytesRead;
    }




    //copy the file if the source closed after that creat a desntnation file and copy it
    int CopyFile(string srcFileName, string destFileName) {

        // Check if the disk is formatted
        if (!is_formated) {
            cout << "Cannot copy a file on an unformatted disk." << endl;
            return -1;
        }

        // Check if the source file exists in MainDir
        if (MainDir.find(srcFileName) == MainDir.end()) {
            cout << "Source file not found: " << srcFileName << endl;
            return -1;
        }


        // Check if the source file is already open
        int srcFd = -1;
        for (int fd = 0; fd < OpenFileDescriptors.size(); ++fd) {
            if (OpenFileDescriptors[fd].isInUse() && OpenFileDescriptors[fd].getFileName() == srcFileName) {
                cout << "Source file is already open." << endl;
                return -1;
            }
        }

        // Check if the destination file is already open
        int destFd = -1;
        for (int fd = 0; fd < OpenFileDescriptors.size(); ++fd) {
            if (OpenFileDescriptors[fd].isInUse() && OpenFileDescriptors[fd].getFileName() == destFileName) {
                cout << "Destination file is already open." << endl;
                return -1;
            }
        }

        // Open the source file for reading
        srcFd = OpenFile(srcFileName);

        // Create the destination file
        destFd = CreateFile(destFileName);


        // Create a buffer for copying data
        const int copy = 1024; // You can adjust the buffer size as needed
        char copy_file[copy];

        int bytesRead;
        int bytesWritten = 0;

        bytesRead = ReadFromFile(srcFd, copy_file, copy);
        int bytesCopied = WriteToFile(destFd, copy_file, bytesRead);

        // Close both the source and destination files
        CloseFile(srcFd);
        CloseFile(destFd);

        return bytesWritten;



    }

    // The RenameFile function is responsible for renaming a file within the file system.
    int RenameFile(string oldFileName, string newFileName) {
        if (!is_formated) {
            cout << "Cannot rename a file on an unformatted disk." << endl;
            return -1; // Return an error code or handle it according to your needs
        }

        if (MainDir.find(oldFileName) == MainDir.end()) {
            cout << "File not found: " << oldFileName << endl;
            return -1; // Return an error code or handle it according to your needs
        }

        if (MainDir.find(newFileName) != MainDir.end()) {
            cout << "File with the new name '" << newFileName << "' already exists." << endl;
            return -1; // Return an error code or handle it accordingly
        }

        // Check if the old file is open
        for (auto& fd : OpenFileDescriptors) {
            if (fd.isInUse() && fd.getFileName() == oldFileName) {
                cout << "File '" << oldFileName << "' is curren5tly open. Close it before renaming." << endl;
                return -1;
            }

        }
        // Get the inode associated with the old file name
        fsInode* fileInode = MainDir[oldFileName];

        // Update the MainDir map with the new file name while preserving the inode
        MainDir[newFileName] = fileInode;

        // Remove the old file name entry from the MainDir map
        MainDir.erase(oldFileName);

        // update the file name in any open file descriptors
        for (auto& fd : OpenFileDescriptors) {
            if (!fd.isInUse()&&fd.getFileName() == oldFileName) {
                fd.setFileName(newFileName);
            }
        }


        return 0;
    }




};

int main() {

    int blockSize;
    int direct_entries;
    string fileName;
    string fileName2;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while(1) {
        cin >> cmd_;
        switch (cmd_)
        {
            case 0:   // exit
                delete fs;
                exit(0);
                break;

            case 1:  // list-file
                fs->listAll();
                break;

            case 2:    // format
                cin >> blockSize;
                cin >> direct_entries;
                fs->fsFormat(blockSize, direct_entries);
                break;

            case 3:    // creat-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd);
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;


            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile( _fd , str_to_write , strlen(str_to_write) );
                break;



            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                fs->ReadFromFile( _fd , str_to_read , size_to_read );
                cout << "ReadFromFile: " << str_to_read << endl;
                break;

            case 8:   // delete file
                cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 9:   // copy file
                cin >> fileName;
                cin >> fileName2;
                fs->CopyFile(fileName, fileName2);
                break;

            case 10:  // rename file
                cin >> fileName;
                cin >> fileName2;
                fs->RenameFile(fileName, fileName2);
                break;



            default:

                break;
        }
    }

}