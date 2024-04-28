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

// #define SYS_CALL
// ============================================================================
class fsInode {
    int fileSize;
    int block_in_use;
    int block_size;
    int directBlock1;
    int directBlock2;
    int directBlock3;

    int singleInDirect;
    int doubleInDirect;
    int doublePointerOffset;

public:
    void setDoublePointerOffset(int doublePointerOffset) {
        fsInode::doublePointerOffset = doublePointerOffset;
    }

public:
    int getDoublePointerOffset() const {
        return doublePointerOffset;
    }


public:
    int getBlockSize() const {
        return block_size;
    }

    void setBlockSize(int blockSize) {
        block_size = blockSize;
    }

    int getDirectBlock1() const {
        return directBlock1;
    }

    void setDirectBlock1(int directBlock1) {
        fsInode::directBlock1 = directBlock1;
    }

    int getDirectBlock2() const {
        return directBlock2;
    }

    void setDirectBlock2(int directBlock2) {
        fsInode::directBlock2 = directBlock2;
    }

    int getDirectBlock3() const {
        return directBlock3;
    }

    void setDirectBlock3(int directBlock3) {
        fsInode::directBlock3 = directBlock3;
    }

    int getSingleInDirect() const {
        return singleInDirect;
    }

    void setSingleInDirect(int singleInDirect) {
        fsInode::singleInDirect = singleInDirect;
    }

    int getDoubleInDirect()  {
        return doubleInDirect;
    }

    void setDoubleInDirect(int doubleInDirect) {
        fsInode::doubleInDirect = doubleInDirect;
    }
    void setBlockInUse(int blockInUse) {
        block_in_use = blockInUse;
    }
public:
    int getBlockInUse()  {
        return block_in_use;
    }



    int getFileSize() const {
        return fileSize;
    }

    void setFileSize(int fileSize) {
        fsInode::fileSize = fileSize;
    }





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
        doublePointerOffset=0;

    }

    int getDirectBlock(int blockIndex) {
        switch (blockIndex) {
            case 0:
                return directBlock1;
            case 1:
                return directBlock2;
            case 2:
                return directBlock3;
            default:
                return -1;
        }
    }

    ~fsInode() {
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



    string getFileName() {
        return file.first;
    }
    string setFileName(string name){// i added this method to help with the renameFile function in the fsDisk class
        file.first=name;
        return name;
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

    ~FileDescriptor(){
        cout<<"";
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk {
    FILE *sim_disk_fd;

    bool is_formated;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize;
    int *BitVector;
    int sizeOfBlock;
    bool nullCheck=false;
    int currentDisk;
    // Unix directories are lists of association structures,
    // each of which contains one filename and one inode number.
    map<string, fsInode*>  MainDir ;

    // OpenFileDescriptors --  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    vector< FileDescriptor > OpenFileDescriptors;


public:
    // ------------------------------------------------------------------------
    fsDisk() {
        //char character;
        sim_disk_fd = fopen( DISK_SIM_FILE , "w+"  );
        assert(sim_disk_fd);
        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
            assert(ret_val == 1);
        }


        fflush(sim_disk_fd);

    }



    // ------------------------------------------------------------------------
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

    // ------------------------------------------------------------------------
    void fsFormat( int blockSize =4, int direct_Enteris_ = 3 ) {
        if(blockSize<=1){

            is_formated=false;
            cout<<"ERROR, invalid block size"<<endl;
            return;
        }


        /*Clearing the disk in case of a multiple formats */

        sim_disk_fd = fopen( DISK_SIM_FILE , "w+"  );
        assert(sim_disk_fd);
        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);


        BitVectorSize=DISK_SIZE/blockSize;//finding the size of the bitVector array
        BitVector=new int[BitVectorSize];//setting the BitVector array
        for (int i = 0; i < BitVectorSize; i++) {
            BitVector[i]=0;//initializing every bit to 0 (block not in use)
        }
        MainDir.clear();
        OpenFileDescriptors.clear();
//        for (auto& iterator : MainDir){
//            fsInode* i_node=iterator.second;
//            i_node->setBlockSize(blockSize);
//        }
        sizeOfBlock=blockSize;
        is_formated=true;
        currentDisk=0;

    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName) {
        if(is_formated==false){
            cout<<"ERROR: DISK HAS NOT BEEN FORMATTED!!!"<<endl;
            return -1;
        }

        for (auto& iterator : MainDir){
            if(iterator.first==fileName){
                cout<<"FILE ALREADY EXISTS!!!!"<<endl;
                return -1;
            }

        }
        for (size_t i = 0; i < OpenFileDescriptors.size(); ++i){
            if (OpenFileDescriptors[i].getFileName() == "") {
                MainDir.insert(make_pair(fileName, OpenFileDescriptors[i].getInode()));
                OpenFileDescriptors[i].setFileName(fileName);
                OpenFileDescriptors[i].getInode()->setFileSize(0);
                OpenFileDescriptors[i].getInode()->setBlockInUse(0);
                OpenFileDescriptors[i].getInode()->setDirectBlock1(-1);
                OpenFileDescriptors[i].getInode()->setDirectBlock2(-1);
                OpenFileDescriptors[i].getInode()->setDirectBlock3(-1);
                OpenFileDescriptors[i].getInode()->setSingleInDirect(-1);
                OpenFileDescriptors[i].getInode()->setDoubleInDirect(-1);
                OpenFileDescriptors[i].getInode()->setDoublePointerOffset(0);
                OpenFileDescriptors[i].setInUse(true);
                int fd = i;
                return fd;
            }
        }
        fsInode* i_node=new fsInode(sizeOfBlock);//creating a new inode for a new file creation
        MainDir.insert(make_pair(fileName,i_node));//adding a pair of file name  and a newly created inode to the map
        FileDescriptor newFd(fileName,i_node);
        OpenFileDescriptors.push_back(newFd);//adding the same pair of inode and name to the open file descriptor vector
        int fd=MainDir.size()-1;//finding the current file descriptor

        return fd;

    }

    // ------------------------------------------------------------------------
    int OpenFile(string FileName ) {
        int index=-1;
        if(is_formated==false){
            cout<<"ERROR: DISK HAS NOT BEEN FORMATTED!!!"<<endl;
            return -1;
        }
        if(MainDir.find(FileName)!=MainDir.end()){//checking if file exists
            for (size_t i = 0; i < OpenFileDescriptors.size(); i++) {
                if (OpenFileDescriptors[i].isInUse()==true && OpenFileDescriptors[i].getFileName() == FileName ) {
                    cout << "Error: File '" << FileName << "' is already open." << endl;
                    return i;
                }
                else if(OpenFileDescriptors[i].isInUse()==false && OpenFileDescriptors[i].getFileName() == FileName  ){//closed file
                    for (size_t i = 0; i < OpenFileDescriptors.size(); ++i) {
                        if (OpenFileDescriptors[i].getFileName() == FileName) {
                            index= i;
                            OpenFileDescriptors[i].setInUse(true);
                        }
                    }
                }
            }
            return index;
        }
        else{
            cout<<"ERROR: FILE DOESN'T EXIST!!!"<<endl;
            return -1;
        }
    }

    // ------------------------------------------------------------------------
    string CloseFile(int fd) {
        if(is_formated==false){
            cout<<"ERROR: DISK HAS NOT BEEN FORMATTED!!!"<<endl;
            return "\nerror";
        }
        if (fd < 0 || static_cast<size_t>(fd) >= OpenFileDescriptors.size()) {
            cout << "Error: Invalid file descriptor." << endl;
            return "";
        }
        string fileName = OpenFileDescriptors[fd].getFileName();
        OpenFileDescriptors[fd].setInUse(false);

        return fileName;
    }
    // ------------------------------------------------------------------------
    char allocateNewBlock() {
        //this is an extra function that helps me return the number of the block
        //(it's index in the bitVector) as a char to be used in the single indirect and the double indirect blocks cases
        int emptyBlockIndex = -1;
        for (int i = 0; i < BitVectorSize; i++) {
            if (BitVector[i] == 0) {
                if(i==0){
                    nullCheck=true;
                }
                emptyBlockIndex = i;
                BitVector[i] = 1;
                break;
            }
        }
        if (emptyBlockIndex ==-1 ) {
            cout << "Can't allocate a block!" << endl;
            return '\0';
        }
        return decToBinary(emptyBlockIndex);
    }
    int WriteToFile(int fd, char *buf, int len ) {
        if(is_formated==false){
            cout<<"ERROR: NEED TO FORMAT FIRST"<<endl;
            return -1;
        }
        if (fd < 0 || static_cast<size_t>(fd) >= OpenFileDescriptors.size()) {
            cout << "Error: Invalid file descriptor." << endl;
            return -1;
        }

        if(OpenFileDescriptors[fd].isInUse()==false){
            cout<<" ERROR: Closed file descriptor. NEEDS TO BE OPEN!!!"<<endl;
            return -1;
        }

        int current_fileSize=OpenFileDescriptors[fd].getInode()->getFileSize();
        int neededBlocks=(len+sizeOfBlock-1)/sizeOfBlock;
        int charsWritten=0;
        int dataPosition=0;
        nullCheck=false;//this is a flag to check when converting block numbers of zero , because
        //the char value of zero is '\0'
        int blockIndex = current_fileSize / sizeOfBlock;\
        int offset = current_fileSize % sizeOfBlock;
        if(current_fileSize<DISK_SIZE){
            if(current_fileSize<sizeOfBlock*3) {
                while (charsWritten < len && OpenFileDescriptors[fd].getInode()->getBlockInUse() < 3) {// the first 3 direct blocks
                    if (OpenFileDescriptors[fd].getInode()->getBlockInUse() <= blockIndex) {
                        int emptyBlockIndex = -1;
                        for (int i = 0; i < BitVectorSize; i++) {
                            if (BitVector[i] == 0) {
                                emptyBlockIndex = i;
                                BitVector[i] = 1;
                                break;
                            }
                        }

                        if (emptyBlockIndex != -1) {
                            if (OpenFileDescriptors[fd].getInode()->getBlockInUse() == 0) {
                                OpenFileDescriptors[fd].getInode()->setDirectBlock1(emptyBlockIndex);
                            } else if (OpenFileDescriptors[fd].getInode()->getBlockInUse() == 1) {
                                OpenFileDescriptors[fd].getInode()->setDirectBlock2(emptyBlockIndex);
                            } else if (OpenFileDescriptors[fd].getInode()->getBlockInUse() == 2) {
                                OpenFileDescriptors[fd].getInode()->setDirectBlock3(emptyBlockIndex);
                            }

                            OpenFileDescriptors[fd].getInode()->setBlockInUse((OpenFileDescriptors[fd].getInode()->getBlockInUse()) + 1);
                        } else {
                            cout << "Can't allocate a block(unable)!!!" << endl;
                            return -1;
                        }
                    }
                    int remainingSpace = sizeOfBlock - offset;
                    int charsToWrite = min(remainingSpace, len - charsWritten);
                    fseek(sim_disk_fd, OpenFileDescriptors[fd].getInode()->getDirectBlock(blockIndex) * sizeOfBlock + offset, SEEK_SET);
                    fwrite(&buf[dataPosition], 1, charsToWrite, sim_disk_fd);

                    currentDisk  +=charsToWrite;
                    charsWritten += charsToWrite;
                    dataPosition += charsToWrite;
                    offset += charsToWrite;
                    current_fileSize+=charsToWrite;
                    if (offset == sizeOfBlock) {
                        offset = 0;
                        blockIndex++;
                    }
                }


            }
            if (OpenFileDescriptors[fd].getInode()->getBlockInUse() == 3 && charsWritten < len && current_fileSize>=3*sizeOfBlock && current_fileSize<(3+sizeOfBlock)*sizeOfBlock && currentDisk<DISK_SIZE) {
                //single indirect block case

                int offset = (current_fileSize) % sizeOfBlock;
                int numBlocksToWrite = (len - charsWritten + sizeOfBlock - 1) / sizeOfBlock;
                char singleIndirectBlock[sizeOfBlock];
                for (int blockCounter = 0; blockCounter < numBlocksToWrite; blockCounter++) {
                    if (OpenFileDescriptors[fd].getInode()->getSingleInDirect() == -1) {
                        //we initialize a block specifically for the block pointers
                        char singleIndirectBlockChar = allocateNewBlock();
                        if (singleIndirectBlockChar == '\0' && nullCheck==false) {
                            return -1;
                        }
                        OpenFileDescriptors[fd].getInode()->setSingleInDirect(singleIndirectBlockChar);
                    }
                    char singleIndirectBlockIndex = OpenFileDescriptors[fd].getInode()->getSingleInDirect();
                    int singleIndirectOffset = (current_fileSize - (sizeOfBlock * 3)) / sizeOfBlock;
                    //here i read the single indirect block to check for next iterations
                    fseek(sim_disk_fd, singleIndirectBlockIndex * sizeOfBlock, SEEK_SET);
                    fread(singleIndirectBlock, 1, sizeOfBlock, sim_disk_fd);

                    //index for the internal block pointers
                    int blockPointerIndex = singleIndirectOffset;
                    while (charsWritten < len && blockPointerIndex < sizeOfBlock) {//here i go over and write to the current block
                        //that is being pointed to by the block pointer
                        int blockOffset ;
                        char blockNumberChar;
                        if (singleIndirectBlock[blockPointerIndex] == '\0') {
                            blockNumberChar = allocateNewBlock();
                            if (blockNumberChar == '\0' && nullCheck==false) {
                                return -1;
                            }


                            singleIndirectBlock[blockPointerIndex] = blockNumberChar;
                            //here I update the single indirect block after getting the block number as a char
                            fseek(sim_disk_fd, singleIndirectBlockIndex * sizeOfBlock, SEEK_SET);
                            fwrite(singleIndirectBlock, 1, sizeOfBlock, sim_disk_fd);
                        } else {
                            blockNumberChar = singleIndirectBlock[blockPointerIndex];
                        }

                        int blockNumber = static_cast<int>(blockNumberChar);
                        //offset of the innermost block
                        blockOffset = ((blockNumber*sizeOfBlock)+(offset))%sizeOfBlock;
                        int charsToWrite = min(sizeOfBlock - blockOffset, len - charsWritten);
                        // writing to innermost block
                        fseek(sim_disk_fd, blockNumber * sizeOfBlock + blockOffset, SEEK_SET);
                        fwrite(&buf[dataPosition], 1, charsToWrite, sim_disk_fd);

                        currentDisk  += charsToWrite;
                        charsWritten += charsToWrite;
                        dataPosition += charsToWrite;
                        current_fileSize+=charsToWrite;
                        blockPointerIndex++;
                        offset += charsToWrite;
                    }
                    offset=0;
                }
            }


            if (current_fileSize>=(3+sizeOfBlock)*sizeOfBlock && charsWritten<len    && current_fileSize<DISK_SIZE && currentDisk<DISK_SIZE){
                //double indirect block case
                //here i implemented the same idea from the single indirect case but more improved and with more depth
                //because it has 2 levels of block pointers
                offset = (current_fileSize) % sizeOfBlock;
                int numBlocksToWrite = (len - charsWritten + sizeOfBlock - 1) / sizeOfBlock;
                char doubleIndirectBlock[sizeOfBlock];
                if (OpenFileDescriptors[fd].getInode()->getDoubleInDirect() == -1) {
                    char doubleIndirectBlockChar = allocateNewBlock();
                    if (doubleIndirectBlockChar == '\0' && nullCheck==false) {
                        return -1;
                    }

                    OpenFileDescriptors[fd].getInode()->setDoubleInDirect(doubleIndirectBlockChar);
                }

                char doubleIndirectBlockIndex = OpenFileDescriptors[fd].getInode()->getDoubleInDirect();
                //here I used a double pointer offset that i initialized in the fsInode class and i only increment it
                //when the current block pointer's single indirect blocks are full
                int doubleIndirectOffset = ((static_cast<int>(doubleIndirectBlockIndex)*sizeOfBlock)+OpenFileDescriptors[fd].getInode()->getDoublePointerOffset())%sizeOfBlock;
                fseek(sim_disk_fd, doubleIndirectBlockIndex * sizeOfBlock, SEEK_SET);
                fread(doubleIndirectBlock, 1, sizeOfBlock, sim_disk_fd);

                int doubleIndirectBlockPointerIndex = doubleIndirectOffset;
                while(charsWritten<len && doubleIndirectBlockPointerIndex<sizeOfBlock){
                    int singleIndirectBlockIndex;
                    if (doubleIndirectBlock[doubleIndirectBlockPointerIndex] == '\0') {
                        char singleIndirectBlockChar = allocateNewBlock();
                        if (singleIndirectBlockChar == '\0' && nullCheck==false) {
                            return -1;
                        }
                        singleIndirectBlockIndex = singleIndirectBlockChar;
                        doubleIndirectBlock[doubleIndirectBlockPointerIndex] = singleIndirectBlockChar;

                        // here i write the uodated double indirect block with the chars of the block pointers
                        fseek(sim_disk_fd, doubleIndirectBlockIndex * sizeOfBlock, SEEK_SET);
                        fwrite(doubleIndirectBlock, 1, sizeOfBlock, sim_disk_fd);
                    }

                    else {
                        singleIndirectBlockIndex = doubleIndirectBlock[doubleIndirectBlockPointerIndex];
                    }


                    int singleIndirectOffset = ((current_fileSize - (sizeOfBlock * 3) - (doubleIndirectOffset * sizeOfBlock * sizeOfBlock)) % (sizeOfBlock * sizeOfBlock)) / sizeOfBlock;
                    char singleIndirectBlock[sizeOfBlock];

                    fseek(sim_disk_fd, singleIndirectBlockIndex * sizeOfBlock, SEEK_SET);
                    fread(singleIndirectBlock, 1, sizeOfBlock, sim_disk_fd);
                    int singleIndirectBlockPointerIndex = singleIndirectOffset;

                    while (charsWritten < len && singleIndirectBlockPointerIndex < sizeOfBlock) {
                        int blockOffset;
                        char blockNumberChar;

                        if (singleIndirectBlock[singleIndirectBlockPointerIndex] == '\0') {
                            blockNumberChar = allocateNewBlock();
                            if (blockNumberChar == '\0' && nullCheck==false) {
                                return -1;
                            }
                            singleIndirectBlock[singleIndirectBlockPointerIndex] = blockNumberChar;
                            //here i update the single indirect block with the char block pointers
                            fseek(sim_disk_fd, singleIndirectBlockIndex * sizeOfBlock, SEEK_SET);
                            fwrite(singleIndirectBlock, 1, sizeOfBlock, sim_disk_fd);
                        } else {
                            blockNumberChar = singleIndirectBlock[singleIndirectBlockPointerIndex];
                        }

                        int blockNumber = static_cast<int>(blockNumberChar);
                        // offset of the innermost block
                        blockOffset = ((blockNumber * sizeOfBlock) + offset) % sizeOfBlock;
                        int charsToWrite = min(sizeOfBlock - blockOffset, len - charsWritten);

                        fseek(sim_disk_fd, blockNumber * sizeOfBlock + blockOffset, SEEK_SET);
                        fwrite(&buf[dataPosition], 1, charsToWrite, sim_disk_fd);

                        currentDisk  += charsToWrite;
                        charsWritten += charsToWrite;
                        dataPosition += charsToWrite;
                        current_fileSize += charsToWrite;
                        singleIndirectBlockPointerIndex++;
                        offset += charsToWrite;
                    }
                    offset = 0;
                    doubleIndirectBlockPointerIndex++;
                    if(singleIndirectBlock[sizeOfBlock-1]!='\0'){
                        OpenFileDescriptors[fd].getInode()->setDoublePointerOffset(OpenFileDescriptors[fd].getInode()->getDoublePointerOffset()+1);
                    }
                }
                offset = 0;
            }
            OpenFileDescriptors[fd].getInode()->setFileSize(current_fileSize);
            return 1;
        }
        else{

            cout<<"DISK IS FULL!!!"<<endl;
            return -1;

        }
    }
    // ------------------------------------------------------------------------
    int DelFile(string FileName) {
        if (is_formated == false) {
            cout << "ERROR: NEED TO FORMAT FIRST" << endl;
            return -1;
        }
        if(MainDir.find(FileName)==MainDir.end()){
            cout << "ERROR: File not found." << endl;
            return -1;
        }
        int fd=-1;
        if(MainDir.find(FileName)!=MainDir.end()){
            for (size_t i = 0; i < OpenFileDescriptors.size(); ++i) {
                if (OpenFileDescriptors[i].getFileName() == FileName) {
                    fd= i;
                }
            }
        }

        if(OpenFileDescriptors[fd].isInUse()==true){
            cout<<"ERROR: FILE NEEDS TO BE CLOSED!!!"<<endl;
            return -1;
        }


        int current_fileSize =OpenFileDescriptors[fd].getInode()->getFileSize();
        int charsDeleted = 0;
        int blockIndex = 0;
        int doubleFlag=-1;

        //direct blocks case
        while (charsDeleted < current_fileSize) {
            int blockNumber = -1;
            if (blockIndex == 0) {
                blockNumber = OpenFileDescriptors[fd].getInode()->getDirectBlock1();
            } else if (blockIndex == 1) {
                blockNumber = OpenFileDescriptors[fd].getInode()->getDirectBlock2();
            } else if (blockIndex == 2) {
                blockNumber = OpenFileDescriptors[fd].getInode()->getDirectBlock3();
            }  if (charsDeleted>=3*sizeOfBlock && charsDeleted<(3+sizeOfBlock)*sizeOfBlock) {
                //the single indirect block case with the same logic from the writeToFile() function
                int counter=0;
                if (OpenFileDescriptors[fd].getInode()->getSingleInDirect() != -1 && counter<=sizeOfBlock) {
                    char singleIndirectBlock[sizeOfBlock];
                    fseek(sim_disk_fd, OpenFileDescriptors[fd].getInode()->getSingleInDirect() * sizeOfBlock, SEEK_SET);
                    fread(singleIndirectBlock, 1, sizeOfBlock, sim_disk_fd);
                    int singleIndirectBlockIndex = blockIndex - 3;
                    if (singleIndirectBlockIndex >= 0 && singleIndirectBlockIndex < sizeOfBlock) {
                        blockNumber = singleIndirectBlock[singleIndirectBlockIndex];

                    }

                    //here I clear the single indirect block pointers themselves
                    char c='\0';
                    fseek(sim_disk_fd, OpenFileDescriptors[fd].getInode()->getSingleInDirect() * sizeOfBlock+singleIndirectBlockIndex, SEEK_SET);
                    fwrite(&c, 1, 1, sim_disk_fd);

                }
            } if(charsDeleted>=(3+sizeOfBlock)*sizeOfBlock && charsDeleted<current_fileSize) {
                // here is the double indirect blocks case (the same logic that i used in the writeToFile() function
                if (OpenFileDescriptors[fd].getInode()->getDoubleInDirect() != -1) {
                    doubleFlag=0;
                    char doubleIndirectBlock[sizeOfBlock];
                    fseek(sim_disk_fd, OpenFileDescriptors[fd].getInode()->getDoubleInDirect() * sizeOfBlock, SEEK_SET);
                    fread(doubleIndirectBlock, 1, sizeOfBlock, sim_disk_fd);
                    int doubleIndirectBlockIndex = (blockIndex - 3 - sizeOfBlock) / sizeOfBlock;
                    if (doubleIndirectBlockIndex >= 0 && doubleIndirectBlockIndex < sizeOfBlock) {
                        char singleIndirectBlockIndex = doubleIndirectBlock[doubleIndirectBlockIndex];
                        if (singleIndirectBlockIndex != '\0') {
                            char singleIndirectBlock[sizeOfBlock];
                            fseek(sim_disk_fd, singleIndirectBlockIndex * sizeOfBlock, SEEK_SET);
                            fread(singleIndirectBlock, 1, sizeOfBlock, sim_disk_fd);
                            int singleIndirectBlockPointerIndex = (blockIndex - 3 - sizeOfBlock) % sizeOfBlock;
                            if (singleIndirectBlockPointerIndex >= 0 && singleIndirectBlockPointerIndex < sizeOfBlock) {
                                blockNumber = singleIndirectBlock[singleIndirectBlockPointerIndex];
                            }

                            //here we clear the single indirect block bytes(clearing the pointers themselves)
                           char c='\0';
                           fseek(sim_disk_fd,  singleIndirectBlockPointerIndex+(singleIndirectBlockIndex * sizeOfBlock), SEEK_SET);
                           fwrite(&c, 1, 1, sim_disk_fd);

                        }

                    }
                }
            }

            if (blockNumber == -1) {
                break; // No more blocks to delete
            }

            //read all of the data from the block and afetr that we change it to nulls and write it again to override the data that was before
            char block[sizeOfBlock];
            fseek(sim_disk_fd, blockNumber * sizeOfBlock, SEEK_SET);
            fread(block, 1, sizeOfBlock, sim_disk_fd);
            memset(block, 0, sizeOfBlock);
            fseek(sim_disk_fd, blockNumber * sizeOfBlock, SEEK_SET);
            fwrite(block, 1, sizeOfBlock, sim_disk_fd);

            charsDeleted += sizeOfBlock;

            for(int i=0;i<BitVectorSize;i++){//making the block available for writing on it again
                if(blockNumber==i){
                    BitVector[i]=0;
                }
            }

            blockIndex++;

        }

        if(doubleFlag==0) {//only enter here after the deletion of the double indirect blocks and it's contents
            char* c[sizeOfBlock];
            memset(c, '\0', sizeOfBlock + 1);
            fseek(sim_disk_fd, OpenFileDescriptors[fd].getInode()->getDoubleInDirect() * sizeOfBlock, SEEK_SET);
            fwrite(&c, 1, sizeOfBlock, sim_disk_fd);
            for(int i=0;i<BitVectorSize;i++){
                if(OpenFileDescriptors[fd].getInode()->getDoubleInDirect() == i){
                    BitVector[i]=0;
                }
            }
        }


        // Resetting private variables
        OpenFileDescriptors[fd].getInode()->setFileSize(0);
        OpenFileDescriptors[fd].getInode()->setBlockInUse(0);
        OpenFileDescriptors[fd].getInode()->setDirectBlock1(-1);
        OpenFileDescriptors[fd].getInode()->setDirectBlock2(-1);
        OpenFileDescriptors[fd].getInode()->setDirectBlock3(-1);
        OpenFileDescriptors[fd].getInode()->setSingleInDirect(-1);
        OpenFileDescriptors[fd].getInode()->setDoubleInDirect(-1);
        OpenFileDescriptors[fd].getInode()->setDoublePointerOffset(0);
        OpenFileDescriptors[fd].setFileName("");
        OpenFileDescriptors[fd].setInUse(false);
        MainDir.erase(FileName);


        return fd;
    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len) {
        if (is_formated==false) {
            cout << "ERROR: NEED TO FORMAT FIRST!!!" << endl;
            memset(buf,'\0',DISK_SIZE);
            return -1;
        }

        if (fd < 0 || static_cast<size_t>(fd) >= OpenFileDescriptors.size()) {
            cout << "Error: Invalid file descriptor." << endl;
            memset(buf,'\0',DISK_SIZE);
            return -1;
        }

        if (OpenFileDescriptors[fd].isInUse() == false) {
            cout << "ERROR: Closed file descriptor. NEEDS TO BE OPEN!!!" << endl;
            memset(buf,'\0',DISK_SIZE);
            return -1;
        }

        //resetting the buffer in case it saved it's value from the previous call to the function
        memset(buf,'\0',len+1);

        int charsRead = 0;
        int dataPosition = 0;
        if(len>OpenFileDescriptors[fd].GetFileSize()){
            len=OpenFileDescriptors[fd].GetFileSize();
        }
        while (charsRead < len) {
            int blockIndex = charsRead / sizeOfBlock;
            int offset = charsRead % sizeOfBlock;

            if (charsRead < 3*sizeOfBlock) {//direct blocks case
                int blockNumber = OpenFileDescriptors[fd].getInode()->getDirectBlock(blockIndex);
                if (blockNumber == -1) {
                    break; //no more data!!!
                }

                int remainingDataInBlock = sizeOfBlock - offset;
                int charsToRead = min(remainingDataInBlock, len - charsRead);

                //this condition is to make sure we only read what's necessary and not read garbage characters
                if (charsRead + charsToRead >= len) {
                    charsToRead = len - charsRead;
                }

                fseek(sim_disk_fd, blockNumber * sizeOfBlock + offset, SEEK_SET);
                fread(&buf[dataPosition], 1, charsToRead, sim_disk_fd);
                charsRead += charsToRead;
                dataPosition += charsToRead;

            }
            if (charsRead<(3+sizeOfBlock)*sizeOfBlock && charsRead>=sizeOfBlock*3) {//single indirect block case with the same
                // logic i used in the writeToFile function
                int singleIndirectBlockIndex = OpenFileDescriptors[fd].getInode()->getSingleInDirect();
                if (singleIndirectBlockIndex == -1) {
                    break; //no more data!!!!
                }

                char singleIndirectBlock[sizeOfBlock];
                fseek(sim_disk_fd, singleIndirectBlockIndex * sizeOfBlock, SEEK_SET);
                fread(singleIndirectBlock, 1, sizeOfBlock, sim_disk_fd);

                int singleIndirectOffset = (charsRead - (sizeOfBlock * 3)) / sizeOfBlock;
                int blockPointerIndex = singleIndirectOffset;
                while (charsRead < len && blockPointerIndex < sizeOfBlock){
                    char blockNumberChar = singleIndirectBlock[blockPointerIndex];
                    if (blockNumberChar == '\0') {
                        break;
                    }

                    int blockNumber = static_cast<int>(blockNumberChar);
                    int blockOffset = ((blockNumber * sizeOfBlock) + offset) % sizeOfBlock;
                    int remainingDataInBlock = sizeOfBlock - blockOffset;
                    int charsToRead = min(remainingDataInBlock, len - charsRead);

                    if (charsRead + charsToRead >= len) {
                        charsToRead = len - charsRead;
                    }

                    fseek(sim_disk_fd, blockNumber * sizeOfBlock + blockOffset, SEEK_SET);
                    fread(&buf[dataPosition], 1, charsToRead, sim_disk_fd);
                    charsRead += charsToRead;
                    dataPosition += charsToRead;
                    offset += charsToRead;

                    if (offset == sizeOfBlock) {
                        offset = 0;
                        blockIndex++;
                        blockPointerIndex++;
                    }
                }
            }
            if (charsRead >= (3 + sizeOfBlock) * sizeOfBlock && charsRead < len) {//double indirect block case
                //initializing offsets
                int doubleIndirectOffset = charsRead - (3 + sizeOfBlock) * sizeOfBlock;
                int doubleIndirectBlockIndex = doubleIndirectOffset / (sizeOfBlock * sizeOfBlock);
                int doubleIndirectBlockOffset = doubleIndirectOffset % (sizeOfBlock * sizeOfBlock);

                char doubleIndirectBlock[sizeOfBlock];
                fseek(sim_disk_fd, OpenFileDescriptors[fd].getInode()->getDoubleInDirect() * sizeOfBlock, SEEK_SET);
                fread(doubleIndirectBlock, 1, sizeOfBlock, sim_disk_fd);

                //here i read block pointers of the double indirect block
                char singleIndirectBlockChar = doubleIndirectBlock[doubleIndirectBlockIndex];
                int singleIndirectBlockIndex = doubleIndirectBlockOffset / sizeOfBlock;
                int singleIndirectOffset = doubleIndirectBlockOffset % sizeOfBlock;
                //here i also read but for the single indirect blocks in the double indirect block
                char singleIndirectBlock[sizeOfBlock];
                fseek(sim_disk_fd, singleIndirectBlockChar * sizeOfBlock, SEEK_SET);
                fread(singleIndirectBlock, 1, sizeOfBlock, sim_disk_fd);

                int singleIndirectBlockPointerIndex = singleIndirectOffset;
                int offset = 0;
                while (charsRead < len && singleIndirectBlockPointerIndex < sizeOfBlock) {
                    //read every char individually
                    char blockNumberChar = singleIndirectBlock[singleIndirectBlockPointerIndex];
                    int blockNumber = static_cast<int>(blockNumberChar);
                    //offset within the innermost block
                    int blockOffset = offset % sizeOfBlock;
                    int charsToRead = min(sizeOfBlock - blockOffset, len - charsRead);
                    if (charsRead + charsToRead >= len) {
                        charsToRead = len - charsRead;
                    }
                    fseek(sim_disk_fd, blockNumber * sizeOfBlock + blockOffset, SEEK_SET);
                    fread(&buf[charsRead], 1, charsToRead, sim_disk_fd);

                    charsRead += charsToRead;
                    offset += charsToRead;
                    singleIndirectBlockPointerIndex++;

                    if (offset == sizeOfBlock) {
                        offset = 0;
                        singleIndirectBlockIndex++;
                    }
                }
            }
        }

        return 1;
    }
    // ------------------------------------------------------------------------
    int CopyFile(string srcFileName, string destFileName) {

        if(is_formated==false){
            cout<<"NEED TO FORMAT FIRST!!!"<<endl;
            return -1;
        }

        if (MainDir.find(srcFileName) == MainDir.end()){
            cout<<"ERROR, the file your trying to copy doesn't exist!!!"<<endl;
            return -1;
        }

        for (size_t i = 0; i < OpenFileDescriptors.size(); ++i) {
            if(OpenFileDescriptors[i].getFileName()==srcFileName && OpenFileDescriptors[i].isInUse()==true){
                cout<<"ERROR: NEED TO CLOSE FILE FIRST!!!"<<endl;
                return -1;
            }
            if(OpenFileDescriptors[i].getFileName()==destFileName && OpenFileDescriptors[i].isInUse()==true){
                cout<<"ERROR: NEED TO CLOSE FILE FIRST!!!"<<endl;
                return -1;
            }

        }

        if (MainDir.find(destFileName) == MainDir.end()) {//check if file is not in disk so we create it
            CreateFile(destFileName);
            OpenFile(srcFileName);
            int fileSize;
            char str_to_read[DISK_SIZE];
            int wroteFlag=-1;//flag for copying the chars from the first file to the 2nd file
            for (int i = 0; i < OpenFileDescriptors.size(); i++) {
                if (OpenFileDescriptors[i].getFileName() == srcFileName) {
                    fileSize = OpenFileDescriptors[i].getInode()->getFileSize();
                    int size_to_read = fileSize;
                    int check=ReadFromFile(i, str_to_read, size_to_read);
                    if(check==-1){
                        cout<<"ERROR IN READING FROM FILE"<<endl;
                        return -1;
                    }
                    CloseFile(i);
                    wroteFlag = 0;
                }

                for (int i = 0; i < OpenFileDescriptors.size(); i++) {
                    if (OpenFileDescriptors[i].getFileName() == destFileName &&wroteFlag==0) {
                        int size_to_read = fileSize;
                        char str_to_write[strlen(str_to_read)];
                        int check= WriteToFile(i, str_to_read, size_to_read);
                        if(check==-1){
                            cout<<"ERROR IN WRITING TO FILE"<<endl;
                            return -1;
                        }
                        CloseFile(i);
                        return 1;
                    }
                }
            }
        }
        else{//if file is existing in disk we override it
            OpenFile(srcFileName);
            DelFile(destFileName);
            CreateFile(destFileName);
            int fileSize;
            char str_to_read[DISK_SIZE];
            int wroteFlag=-1;
            for (int i = 0; i < OpenFileDescriptors.size(); i++) {
                if (OpenFileDescriptors[i].getFileName() == srcFileName) {
                    fileSize = OpenFileDescriptors[i].getInode()->getFileSize();
                    int size_to_read = fileSize;
                    int check=ReadFromFile(i, str_to_read, size_to_read);
                    if(check==-1){
                        cout<<"ERROR IN READING FROM FILE"<<endl;
                    }
                    CloseFile(i);
                    wroteFlag = 0;
                }
                for (int i = 0; i < OpenFileDescriptors.size(); i++) {
                    if (OpenFileDescriptors[i].getFileName() == destFileName && wroteFlag==0) {
                        int size_to_read = fileSize;
                        char str_to_write[strlen(str_to_read)];
                        int check= WriteToFile(i, str_to_read, size_to_read);
                        if(check==-1){
                            cout<<"ERROR IN WRITING TO FILE"<<endl;
                            return -1;
                        }
                        CloseFile(i);
                    }
                }
            }

        }

        return 1;
    }
    // ------------------------------------------------------------------------
    int RenameFile(string oldFileName, string newFileName) {

        if(is_formated==false){
            cout<<"ERROR: NEED TO FORMAT FIRST!!!"<<endl;
            return -1;
        }

        if (MainDir.find(newFileName) != MainDir.end()){
            cout<<"ERROR: CANNOT RENAME TO EXISTING NAME!!!"<<endl;
            return -1;
        }
        if (MainDir.find(oldFileName) != MainDir.end()) {
            fsInode* oldInode = MainDir[oldFileName];

            for (size_t i = 0; i < OpenFileDescriptors.size(); i++) {
                if (OpenFileDescriptors[i].isInUse() == false && OpenFileDescriptors[i].getInode() == oldInode) {
                    OpenFileDescriptors[i].setFileName(newFileName);
                    MainDir.erase(oldFileName);
                    MainDir[newFileName] = oldInode;
                    return 1;
                }
            }
            cout<<"ERROR: FILE IS OPEN AND NEEDS TO BE CLOSED TO ABLE TO RENAME!!!"<<endl;
            return -1;

        }
        else{
            cout<<"ERROR: FILE DOESN'T EXIST!!!"<<endl;
            return -1;
        }
    }

    ~fsDisk() {
        if (sim_disk_fd != nullptr) {
            fclose(sim_disk_fd);
            sim_disk_fd = nullptr;
        }

        if (BitVector != nullptr) {
            delete[] BitVector;
            BitVector = nullptr;
        }

        for (auto& entry : MainDir) {
            delete entry.second;
        }
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
