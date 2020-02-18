//
//  test.cpp
//  CUGL
//
//  Created by Walker White on 7/11/16.
//  Copyright © 2016 Game Design Initiative at Cornell. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <cugl/cugl.h>

#include "TCUMathTest.h"
#include "TCU2DTest.h"

#include <Accelerate/Accelerate.h>

void testBinary() {
    CULog("Writing to File");
    std::shared_ptr<cugl::BinaryWriter> writer = cugl::BinaryWriter::alloc("binary.b");
    writer->write('c');
    writer->writeSint16(-2);
    writer->writeUint16(2);
    writer->writeSint32(-20);
    writer->writeUint32(20);
    writer->writeSint64(-200);
    writer->writeUint64(200);
    writer->writeFloat(1.234f);
    writer->writeDouble(4.567);
    
    const char* s = "Hello";
    writer->write(s,4,1);
    
    const Uint8 a1[5] = {0,1,2,3,4};
    writer->write(a1,4,1);

    const Sint16 a2[5] = {0,-1,2,-3,4};
    writer->write(a2,4,1);

    const Uint16 a3[5] = {0,1,2,3,4};
    writer->write(a3,4,1);

    const Sint32 a4[5] = {0,-10,20,-30,40};
    writer->write(a4,4,1);
    
    const Uint32 a5[5] = {0,10,20,30,40};
    writer->write(a5,4,1);

    const Sint64 a6[5] = {0,-100,200,-300,400};
    writer->write(a6,4,1);
    
    const Uint64 a7[5] = {0,100,200,300,400};
    writer->write(a7,4,1);

    const float a8[5] = {0.0f,0.1f,0.2f,0.3f,0.4f};
    writer->write(a8,4,1);
    
    const double a9[5] = {0,0.11,0.22,0.33,0.44};
    writer->write(a9,4,1);
    
    writer->close();
    
    CULog("Reading from File");
    std::shared_ptr<cugl::BinaryReader> reader = cugl::BinaryReader::alloc("binary.b");
    CULog("%c",reader->readChar());
    CULog("%d",reader->readSint16());
    CULog("%d",reader->readUint16());
    CULog("%d",reader->readSint32());
    CULog("%d",reader->readUint32());
    CULog("%lld",reader->readSint64());
    CULog("%lld",reader->readUint64());
    CULog("%.3f",reader->readFloat());
    CULog("%.3f",reader->readDouble());

    size_t amt;
    
    char* b0 = (char*)malloc(8);
    amt = reader->read(b0,4,0);
    b0[amt] = 0;
    CULog("String is %s",b0);
    free(b0);

    Uint8* b1 = (Uint8*)malloc(8);
    amt = reader->read(b1,4,0);
    CULog("String is %s",cugl::to_string(b1,amt).c_str());
    free(b1);

    Sint16* b2 = (Sint16*)malloc(8*2);
    amt = reader->read(b2,4,0);
    CULog("String is %s",cugl::to_string(b2,amt).c_str());
    free(b2);

    Uint16* b3 = (Uint16*)malloc(8*2);
    amt = reader->read(b3,4,0);
    CULog("String is %s",cugl::to_string(b3,amt).c_str());
    free(b3);

    Sint32* b4 = (Sint32*)malloc(8*4);
    amt = reader->read(b4,4,0);
    CULog("String is %s",cugl::to_string(b4,amt).c_str());
    free(b4);
    
    Uint32* b5 = (Uint32*)malloc(8*4);
    amt = reader->read(b5,4,0);
    CULog("String is %s",cugl::to_string(b5,amt).c_str());
    free(b5);

    Sint64* b6 = (Sint64*)malloc(8*8);
    amt = reader->read(b6,4,0);
    CULog("String is %s",cugl::to_string(b6,amt).c_str());
    free(b6);
    
    Uint64* b7 = (Uint64*)malloc(8*8);
    amt = reader->read(b7,4,0);
    CULog("String is %s",cugl::to_string(b7,amt).c_str());
    free(b7);

    float* b8 = (float*)malloc(8*4);
    amt = reader->read(b8,4,0);
    CULog("String is %s",cugl::to_string(b8,amt).c_str());
    free(b8);
    
    double* b9 = (double*)malloc(8*8);
    amt = reader->read(b9,5,0);
    CULog("String is %s",cugl::to_string(b9,amt).c_str());
    free(b9);
    
    CULog("Ready: %d",reader->ready());
    reader->close();

}

class Item {
protected:
    int _value;
public:
    Item() : _value(0) { CULog("Allocating item"); }
    ~Item() { CULog("Disposing item"); }
    
    void reset() { CULog("Reseting item"); _value = 0; }
    
    int getValue() const { return _value; }
    void setValue(int value) { _value = value; }
};

void testFree() {
    std::shared_ptr<cugl::GreedyFreeList<Item>> list = cugl::GreedyFreeList<Item>::alloc(2);
    
    Item* p = list->malloc();
    p->setValue(4);
    
    Item* q = list->malloc();
    q->setValue(6);

    Item* r = list->malloc();
    r->setValue(8);

    list = nullptr;
    
}

void testThread() {
    std::shared_ptr<cugl::ThreadPool> pool = cugl::ThreadPool::alloc(2);
    pool->addTask([=] { CULog("Thread 1"); });
    pool->addTask([=] { CULog("Thread 2"); });
    pool->addTask([=] { CULog("Thread 3"); });
    pool->addTask([=] { CULog("Thread 4"); });
    pool = nullptr;
}


int main(int argc, char * argv[]) {
    cugl::Application app;
    app.setName("Unit Test");
    app.setOrganization("GDIAC");
    if (!app.init()) {
        return 1;
    }
    
    app.onStartup();

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    CULog("Little endian");
#else
    CULog("Big endian");
#endif
    
#if defined CU_MATH_VECTOR_NEON64
    CULog("Neon64 Vectorization Support");
#elif defined CU_MATH_VECTOR_SSE
    CULog("SSE Vectorization Support");
#else
    CULog("No Vectorization Support");
#endif
    
    cugl::mathUnitTest();

    //cugl::sceneUnitTest();
    //testBinary();
    //testFree();
    //testThread();
    
    app.quit();
    app.onShutdown();
    return 0;
}
