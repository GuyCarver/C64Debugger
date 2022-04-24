
#pragma once

#include "types.h"
#include "json/json.hpp"

class Response;

namespace Code
{

//----------------------------------------------------------------
///Save data to Json
void ToJson( nlohmann::json &arData );

//----------------------------------------------------------------
///Load data from Json
void FromJson( nlohmann::json &arData );

//----------------------------------------------------------------
void FromResponse( const Response &arResponse );

//----------------------------------------------------------------
///Refresh data
void Refresh(  );

//----------------------------------------------------------------
///Redo disassembly
void UpdateDisView(  );

//----------------------------------------------------------------
///Enable window
void Enable(  );

//----------------------------------------------------------------
///Set new address for window
void SetAddress( uint16_t aAddress );

//----------------------------------------------------------------
///Set new address if Following Instruction Pointer
void NewIP( uint16_t aAddress );

//----------------------------------------------------------------
///Draw window
void Display( bool abInputEnabled );

}	//namespace Code
