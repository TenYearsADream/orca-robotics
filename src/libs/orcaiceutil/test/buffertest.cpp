/*
 * Orca Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2007 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This copy of Orca is licensed to you under the terms described in the
 * ORCA_LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <orcaiceutil/buffer.h>

using namespace std;

int main(int argc, char * argv[])
{
    orcaiceutil::Buffer<double> buffer(-1, orcaiceutil::BufferTypeCircular);
    double data = 20.0;
    double copy = -1.0;

    cout<<"testing default constructor and depth() and type() ... ";
    if ( buffer.depth()!=-1 || buffer.type()!=orcaiceutil::BufferTypeCircular ) {
        cout<<"failed. depth: exp=-1 got="<<buffer.depth()<<" type: exp="<<(int)orcaiceutil::BufferTypeCircular<<" got="<<(int)buffer.type()<<endl;
        return EXIT_FAILURE;
    }
    cout<<"ok"<<endl;

    cout<<"testing get() with empty buffer ... ";
    // call get on an empty stomach
    try
    {
        buffer.get( copy );
        cout<<"failed. empty buffer, should've caught exception"<<endl;
        return EXIT_FAILURE;
    }
    catch ( const orcaiceutil::Exception & )
    {
        ; // ok
    }
    cout<<"ok"<<endl;
    
    cout<<"testing getAndPop() with empty buffer ... ";
    try
    {
        buffer.getAndPop( data );
        cout<<"failed. empty buffer, should've caught exception"<<endl;
        return EXIT_FAILURE;
    }
    catch ( const orcaiceutil::Exception & )
    {
        ; // ok
    }
    cout<<"ok"<<endl;

    cout<<"testing getNext() with empty buffer ... ";
    if ( buffer.getNext( data, 50 )==0 ) {
        cout<<"failed. not expecting anybody setting the buffer"<<endl;
        return EXIT_FAILURE;
    }
    cout<<"ok"<<endl;

    cout<<"testing getAndPopNext() with empty buffer ... ";
    if ( buffer.getAndPopNext( data, 50 )==0 ) {
        cout<<"failed. not expecting anybody setting the proxy"<<endl;
        return EXIT_FAILURE;
    }
    cout<<"ok"<<endl;

    cout<<"testing isEmpty() and size() with empty buffer ... ";
    if ( !buffer.isEmpty() || buffer.size()!=0 ) {
        cout<<"failed. expecting an empty buffer."<<endl;
        return EXIT_FAILURE;
    }
    cout<<"ok"<<endl;

    cout<<"testing push() ... ";
    for ( int i=0; i<3; ++i ) {
        buffer.push( data );
    }
    cout<<"ok"<<endl;

    cout<<"testing isEmpty() and size() ... ";
    if ( buffer.isEmpty() || buffer.size()!=3 ) {
        cout<<"failed on line "<<__LINE__<<": expecting an empty buffer of size 3."<<endl;
        cout<<"TRACE(buffertest.cpp): buffer.size(): " << buffer.size() << endl;
        
        return EXIT_FAILURE;
    }
    cout<<"ok"<<endl;

    cout<<"testing get() ... ";
    try
    {
        buffer.get( copy );
    }
    catch ( const orcaiceutil::Exception & )
    {
        cout<<"failed. should be a non-empty buffer."<<endl;
        return EXIT_FAILURE;
    }
    if ( data!=copy )
    {
        cout<<"failed. expecting an exact copy of the data."<<endl;
        cout<<"\tin="<<data<<" out="<<copy<<endl;
        return EXIT_FAILURE;
    }
    if ( buffer.isEmpty() || buffer.size()!=3 ) {
        cout<<"failed on line "<<__LINE__<<": expecting an empty buffer of size 3."<<endl;
        return EXIT_FAILURE;
    }
    cout<<"ok, size="<<buffer.size()<<endl;
    
    cout<<"testing getAndPop()... ";
    try
    {
        int size = buffer.size();
        for ( int i=0; i < size; i++ )
            buffer.getAndPop( data );
    }
    catch ( const orcaiceutil::Exception & )
    {
        cout<<"failed. should be a non-empty buffer."<<endl;
        return EXIT_FAILURE;
    }
    if ( !buffer.isEmpty() || buffer.size()!=0 ) {
        cout<<"failed. expecting an empty buffer."<<endl;
        return EXIT_FAILURE;
    }
    cout<<"ok"<<endl;
    
    cout<<"testing circular buffer behavior ...";
    buffer.configure( 1, orcaiceutil::BufferTypeCircular );
    //this fills the buffer
    buffer.push( 0 );
    // this should over-write
    buffer.push( 1 );
    buffer.get( data );
    if ( data != 1 ) {
        cout<<"failed. second push should overwrite: expected=1, got="<<data<<endl;
        return EXIT_FAILURE;
    }
    cout<<"ok"<<endl;

    cout<<"testing queue buffer behavior ...";
    buffer.configure( 1, orcaiceutil::BufferTypeQueue );
    //this fills the buffer
    buffer.push( 0 );
    // this should be ignored
    buffer.push( 1 );
    buffer.get( data );
    if ( data != 0 ) {
        cout<<"failed. second push should not overwrite: expected=0, got="<<data<<endl;
        return EXIT_FAILURE;
    }
    cout<<"ok"<<endl;

    cout<<"testing configure() with orcaiceutil::BufferTypeCircular ... ";
    buffer.configure( 300, orcaiceutil::BufferTypeCircular );
    for ( int i=0; i<400; ++i ) {
        buffer.push( data );
    }
    if ( buffer.isEmpty() || buffer.size()!=300 ) {
        cout<<"failed. expecting a buffer of size 300."<<endl;
        return EXIT_FAILURE;
    }
    // todo: test where the last data actually went.
    cout<<"ok"<<endl;

    cout<<"testing purge()... ";
    buffer.purge();
    if ( !buffer.isEmpty() || buffer.size()!=0 ) {
        cout<<"failed. expecting an empty buffer."<<endl;
        return EXIT_FAILURE;
    }
    cout<<"ok"<<endl;

    cout<<"testing configure() with orcaiceutil::BufferTypeQueue ... ";
    buffer.configure( 300, orcaiceutil::BufferTypeQueue );
    for ( int i=0; i<400; ++i ) {
        buffer.push( data );
    }
    if ( buffer.isEmpty() || buffer.size()!=300 ) {
        cout<<"failed. expecting a buffer of size 300."<<endl;
        return EXIT_FAILURE;
    }
    // todo: test where the last data actually went.
    cout<<"ok"<<endl;

    cout<<"testing getNext() ... ";
    if ( buffer.getNext( data, 50 )!=0 ) {
        cout<<"failed. expected to get data"<<endl;
        return EXIT_FAILURE;
    }
    cout<<"ok"<<endl;

    cout<<"testing getAndPopNext() ... ";
    if ( buffer.getAndPopNext( data, 50 )!=0 ) {
        cout<<"failed. expected to get data"<<endl;
        return EXIT_FAILURE;
    }
    cout<<"ok"<<endl;

    cout<<"testing pop() ... ";
    for ( int i=0; i<400; ++i ) {
        buffer.pop();
    }
    if ( !buffer.isEmpty() || buffer.size()!=0 ) {
        cout<<"failed. expecting an empty buffer."<<endl;
        return EXIT_FAILURE;
    }
    // todo: test where the last data actually went.
    cout<<"ok"<<endl;
    
    
    return EXIT_SUCCESS;
}