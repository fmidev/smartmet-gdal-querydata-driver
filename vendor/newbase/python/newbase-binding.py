#! /usr/bin/python3

import pybindgen
import sys
import re

from pybindgen import *

MODULE_NAME = 'newbase'

import re

def extract_enum_from_header(file_path, enum_name, begin_with = None):
    enums = []

    # Regex to match enums and their bodies
    enum_begin = re.compile(r'^\s*enum\s+(\w+)\s*({\s*)?$')
    # Regex to match enum members with optional values and optional comments
    enum_value_pattern = re.compile(r'(\w+)\s*(?:=\s*(\S+))?\s*(?:\/\/[^\n]*|\/\*.*?\*\/)?')

    active = False
    end_found = False

    enum_members = None

    with open(file_path, 'r') as file:

        enum_members = []

        for line in file:
            line = line.strip()
            line = re.sub(r'//.*', '', line)
            line = re.sub(r'/\*.*\*/', '', line)

            if active:
                if '}' in line:
                    end_found = True
                    line.replace(r'}.*', '')

                parts = line.split(r',')

                for part in parts:
                    # Strip possible comma from the end of the line
                    part = part.replace(r',.*', '')
                    match = enum_value_pattern.match(part)
                    if match:
                        member_name = match.group(1)
                        member_value = match.group(2)
                        if begin_with is None or member_name.startswith(begin_with):
                            if member_value is None:
                                enum_members.append((member_name))
                            else:
                                enum_members.append((member_name, member_value))
                if end_found:
                    active = False
            else:
                if not end_found:
                    match = enum_begin.match(line)
                    if match:
                        if match.group(1) == enum_name:
                            active = True
    return enum_members

# includes

def addIncludes(mod):

    mod.add_include('"NFmiGlobals.h"')
    mod.add_include('"NFmiPoint.h"')
    mod.add_include('"NFmiQueryInfo.h"')
    mod.add_include('"NFmiQueryData.h"')
    mod.add_include('"NFmiTimeDescriptor.h"')
    mod.add_include('"NFmiParamDescriptor.h"')
    mod.add_include('"NFmiHPlaceDescriptor.h"')
    mod.add_include('"NFmiVPlaceDescriptor.h"')
    mod.add_include('"NFmiLocation.h"')
    mod.add_include('"NFmiIndividual.h"')
    mod.add_include('"NFmiSortable.h"')
    mod.add_include('"NFmiString.h"')
    mod.add_include('"NFmiMetTime.h"')
    mod.add_include('"NFmiTime.h"')
    mod.add_include('"NFmiStaticTime.h"')
    mod.add_include('"NFmiDataIdent.h"')
    mod.add_include('"NFmiParam.h"')
    mod.add_include('"NFmiProducer.h"')
    mod.add_include('"NFmiParameterName.h"')
    mod.add_include('"NFmiGrid.h"')
    mod.add_include('"NFmiGridBase.h"')
    mod.add_include('"NFmiFastQueryInfo.h"')
    mod.add_include('"NFmiStreamQueryData.h"')
    mod.add_include('"NFmiStation.h"')
    mod.add_include('"NFmiLocationBag.h"')
    mod.add_include('"NFmiStationBag.h"')
    mod.add_include('"NFmiArea.h"')
    mod.add_include('"NFmiLatLonArea.h"')
    mod.add_include('"NFmiDataDescriptor.h"')
    mod.add_include('"NFmiParamBag.h"')
    mod.add_include('"NFmiVoidPtrList.h"')
    mod.add_include('"NFmiTimeList.h"')
    mod.add_include('"NFmiTimeBag.h"')
    mod.add_include('"NFmiTimePeriod.h"')
    mod.add_include('"NFmiQueryDataUtil.h"')
    mod.add_include('"NFmiKKJArea.h"')
    mod.add_include('"NFmiYKJArea.h"')
    mod.add_include('"NFmiLevel.h"')
    mod.add_include('"NFmiEnumConverter.h"')
    mod.add_include('"NFmiLevelBag.h"')
    mod.add_include('"NFmiGdalArea.h"')
    mod.add_include('"NFmiAzimuthalArea.h"')
    mod.add_include('"NFmiStereographicArea.h"')

def includeClasses(mod):
    
    # classes

    NFmiSortable = mod.add_class('NFmiSortable')
    NFmiSize = mod.add_class('NFmiSize')
    NFmiString = mod.add_class('NFmiString', parent=NFmiSortable)
    NFmiIndividual = mod.add_class('NFmiIndividual', parent=NFmiSortable)
    NFmiParam = mod.add_class('NFmiParam', parent=NFmiIndividual)
    NFmiDataIdent = mod.add_class('NFmiDataIdent')
    NFmiStaticTime = mod.add_class('NFmiStaticTime', parent=NFmiSortable)
    NFmiTime = mod.add_class('NFmiTime', parent=NFmiStaticTime)
    NFmiMetTime = mod.add_class('NFmiMetTime', parent=NFmiTime)
    NFmiPoint = mod.add_class('NFmiPoint')
    NFmiQueryData = mod.add_class('NFmiQueryData')
    NFmiDataDescriptor = mod.add_class('NFmiDataDescriptor')
    NFmiTimeDescriptor = mod.add_class('NFmiTimeDescriptor', parent=NFmiDataDescriptor)
    NFmiParamDescriptor = mod.add_class('NFmiParamDescriptor', parent=NFmiDataDescriptor)
    NFmiHPlaceDescriptor = mod.add_class('NFmiHPlaceDescriptor', parent=NFmiDataDescriptor)
    NFmiVPlaceDescriptor = mod.add_class('NFmiVPlaceDescriptor', parent=NFmiDataDescriptor)
    NFmiLocation = mod.add_class('NFmiLocation', parent=NFmiIndividual)
    NFmiQueryInfo = mod.add_class('NFmiQueryInfo')
    NFmiFastQueryInfo = mod.add_class('NFmiFastQueryInfo', parent=NFmiQueryInfo)
    NFmiStreamQueryData = mod.add_class('NFmiStreamQueryData')
    NFmiStation = mod.add_class('NFmiStation', parent=NFmiLocation)
    NFmiLocationBag = mod.add_class('NFmiLocationBag', parent=NFmiSize)
    NFmiStationBag = mod.add_class('NFmiStationBag', parent=NFmiLocationBag)
    NFmiArea = mod.add_class('NFmiArea')
    NFmiLatLonArea = mod.add_class('NFmiLatLonArea',parent=NFmiArea)
    NFmiProducer = mod.add_class('NFmiProducer',parent=NFmiIndividual)
    NFmiParamBag = mod.add_class('NFmiParamBag',parent=NFmiSize)
    NFmiVoidPtrList = mod.add_class('NFmiVoidPtrList')
    NFmiTimeList = mod.add_class('NFmiTimeList')
    NFmiTimeBag = mod.add_class('NFmiTimeBag',parent=NFmiSize)
    NFmiTimePerioid = mod.add_class('NFmiTimePerioid')
    NFmiQueryDataUtil = mod.add_class('NFmiQueryDataUtil')
    NFmiGridBase = mod.add_class('NFmiGridBase')
    NFmiGrid = mod.add_class('NFmiGrid',parent=NFmiGridBase)
    NFmiKKJArea =  mod.add_class('NFmiKKJArea',parent=NFmiArea)
    NFmiYKJArea = mod.add_class('NFmiYKJArea',parent=NFmiKKJArea)
    NFmiLevel = mod.add_class('NFmiLevel',parent=NFmiIndividual)
    NFmiEnumConverter = mod.add_class('NFmiEnumConverter')
    NFmiLevelBag = mod.add_class('NFmiLevelBag')
    NFmiGdalArea = mod.add_class('NFmiGdalArea',parent=NFmiArea)
    NFmiAzimuthalArea = mod.add_class('NFmiAzimuthalArea',parent=NFmiArea)
    NFmiStereographicArea = mod.add_class('NFmiStereographicArea',parent=NFmiAzimuthalArea)

    # constructors

    NFmiQueryInfo.add_constructor([param('std::string&','fileName',is_const=True)])
    NFmiQueryInfo.add_constructor([param('double','theInfoVersion',default_value='7')])
    NFmiQueryInfo.add_constructor([param('NFmiQueryData*','data',transfer_ownership=False),
                                   param('NFmiParamDescriptor*', 'theParamDescriptor',default_value='0',transfer_ownership=False),
                                   param('NFmiTimeDescriptor*', 'theTimeDescriptor',default_value='0',transfer_ownership=False),
                                   param('NFmiHPlaceDescriptor*', 'theHPlaceDescriptor',default_value='0',transfer_ownership=False),
                                   param('NFmiVPlaceDescriptor*', 'theVPlaceDescriptor',default_value='0',transfer_ownership=False)])

    NFmiPoint.add_copy_constructor()
    NFmiPoint.add_constructor([])
    NFmiPoint.add_constructor([param('double', 'theX'),param('double','theY')])

    NFmiStreamQueryData.add_constructor([])
    NFmiStreamQueryData.add_constructor([param('NFmiQueryData*', 'theQueryData', transfer_ownership=False), param('bool','isOwnerData',default_value='false')])

    NFmiQueryData.add_constructor([param('const std::string&', 'thePath'), param('bool', 'theMemoryMapFlag', default_value='true')])
    NFmiQueryData.add_constructor([param('const NFmiQueryInfo&','theInfo')])
    NFmiQueryData.add_constructor([param('const NFmiQueryData&','theData')])

    NFmiFastQueryInfo.add_constructor([param('const NFmiQueryInfo&', 'theInfo')])
    NFmiFastQueryInfo.add_constructor([param('NFmiQueryData*','data',transfer_ownership=False),
                                       param('NFmiParamDescriptor*', 'theParamDescriptor',default_value='0',transfer_ownership=False),
                                       param('NFmiTimeDescriptor*', 'theTimeDescriptor',default_value='0',transfer_ownership=False),
                                       param('NFmiHPlaceDescriptor*', 'theHPlaceDescriptor',default_value='0',transfer_ownership=False),
                                       param('NFmiVPlaceDescriptor*', 'theVPlaceDescriptor',default_value='0',transfer_ownership=False)])

    NFmiFastQueryInfo.add_constructor([param('const NFmiParamDescriptor&', 'theParamDescriptor'),
                                       param('const NFmiTimeDescriptor&', 'theTimeDescriptor'),
                                       param('const NFmiHPlaceDescriptor&', 'theHPlaceDescriptor'), # Has default value in c++
                                       param('const NFmiVPlaceDescriptor&', 'theVPlaceDescriptor'), # Has default value in c++
				       param('double','theInfoVersion',default_value='7.')])

    NFmiString.add_constructor([param('const NFmiString&', 'aString')])
    NFmiString.add_constructor([param('const std::string&', 'str')])
    NFmiString.add_constructor([param('const char *', 'aText')])
    #NFmiString.add_constructor([param('const unsigned char *', 'aText')])
   
    # NFmiArea.add_constructor([])

    NFmiLatLonArea.add_constructor([])
    NFmiLatLonArea.add_constructor([param('const NFmiPoint&', 'theBottomLeftLatLon'),param('const NFmiPoint&', 'theTopRightLatLon'),param('const NFmiPoint&', 'theTopLeftXY', default_value='NFmiPoint(0.,0.)'),param('const NFmiPoint&', 'theBottomRightXY', default_value='NFmiPoint(1.,1.)')])

    NFmiYKJArea.add_constructor([])
    NFmiYKJArea.add_constructor([param('const NFmiPoint&', 'theBottomLeftLatLon'),param('const NFmiPoint&', 'theTopRightLatLon'),param('const NFmiPoint&', 'theTopLeftXY', default_value='NFmiPoint(0.,0.)'),param('const NFmiPoint&', 'theBottomRightXY', default_value='NFmiPoint(1.,1.)')])
    NFmiYKJArea.add_constructor([param('const NFmiPoint&', 'theBottomLeftWorldXY'),param('const NFmiPoint&', 'theTopRightWorldXy'),param('bool&','IsWorldXYInMeters'),param('const NFmiPoint&', 'theTopLeftXY', default_value='NFmiPoint(0.,0.)'),param('const NFmiPoint&', 'theBottomRightXY', default_value='NFmiPoint(1.,1.)')])

    NFmiGrid.add_copy_constructor()
    NFmiGrid.add_constructor([param('const NFmiArea*', 'theArea', default_value='0',transfer_ownership=False),
                             param('unsigned long', 'theXNumber', default_value='0'),
                             param('unsigned long', 'theYNumber', default_value='0'),
                             param('FmiDirection', 'theStartingCorner', default_value='kBottomLeft'),
                             param('FmiInterpolationMethod','theInterpolationMethod', default_value='kLinearly')])   

    NFmiStation.add_copy_constructor()
    NFmiStation.add_constructor([param('unsigned long', 'theIdent'),
                                 param('const NFmiString&', 'theName', default_value = '"undefined"'),
				 param('double', 'theLongitude', default_value = 'static_cast<double> (kFloatMissing)'),
				 param('double', 'theLatitude', default_value = 'static_cast<double> (kFloatMissing)'),
				 param('double', 'theMaxDistance', default_value = 'static_cast<double> (kFloatMissing)'),
				 param('FmiStationType', 'theIdentType', default_value = 'kWMO')])

    NFmiLocationBag.add_constructor([])
    NFmiStationBag.add_constructor([])
    
    NFmiVPlaceDescriptor.add_constructor([])
    NFmiVPlaceDescriptor.add_constructor([param('const NFmiVPlaceDescriptor&', 'theVPlaceDescriptor')])
    NFmiVPlaceDescriptor.add_constructor([param('NFmiLevelBag&', 'theLevelBag')])
   
    NFmiHPlaceDescriptor.add_constructor([])
    NFmiHPlaceDescriptor.add_constructor([param('const NFmiHPlaceDescriptor&', 'theHPlaceDescriptor')])
    NFmiHPlaceDescriptor.add_constructor([param('const NFmiLocationBag&','theLocationBag'),
                                          param('FmiStationType','theSelectedType',default_value='kWMO'),
					  param('unsigned long','theMaxNumberOfSources',default_value='0')])

    NFmiHPlaceDescriptor.add_constructor([param('const NFmiLocationBag&','theLocationBag'),
                                          param('const NFmiArea&','theArea'),
					  param('FmiStationType','theSelectedType',default_value='kAll'),
					  param('unsigned long','theMaxNumberOfSources',default_value='0')])
					  
    NFmiHPlaceDescriptor.add_constructor([param('const NFmiHPlaceDescriptor&','theHPlaceDescriptor')])

    NFmiHPlaceDescriptor.add_constructor([param('const NFmiGrid &', 'theGrid'),
                                          param('FmiStationType', 'theSelectedType', default_value='kLatLon'),
                                          param('unsigned long', 'theMaxNumberOfSources', default_value='0')])



    NFmiParamDescriptor.add_constructor([param('const NFmiParamDescriptor&', 'theParamDescriptor')])
    NFmiParamDescriptor.add_constructor([param('const NFmiParamBag&','theParamBag'),param('bool','interpolate',default_value='false')])
 
    NFmiTimeDescriptor.add_constructor([param('const NFmiTimeDescriptor&', 'theTimeDescriptor')])
    NFmiTimeDescriptor.add_constructor([param('const NFmiMetTime&','theOriginTime'),
                                        param('const NFmiTimeList&','theTimeList'),
					param('FmiTimeLocalzation','theIsLocalTime',default_value='kUTC'),
					param('bool','theIsInterpolation',default_value='false')])
					
    NFmiTimeDescriptor.add_constructor([param('const NFmiMetTime&','theOriginTime'),
                                        param('const NFmiTimeBag&','theValidTimeBag'),
					param('FmiTimeLocalzation','theIsLocalTime',default_value='kUTC'),
					param('bool','theIsInterpolation',default_value='false')])
    
    NFmiTimeList.add_constructor([])
    
    NFmiParam.add_constructor([param('const NFmiParam&','theParam')])
      
    NFmiParam.add_constructor([param('unsigned long','theIdent'),
                               param('const NFmiString&','theName',default_value='"Koiranpentu"'),
			       param('double','theMinValue',default_value='kFloatMissing'),
			       param('double','theMaxValue',default_value='kFloatMissing'),
			       param('double','theScale',default_value='kFloatMissing'), # This should be float !
			       param('double','theBase',default_value='kFloatMissing'), # This should be float !
			       param('const NFmiString&','itsPrecision',default_value='"%.1f"'),
			       param('FmiInterpolationMethod','theInterpolationMethod',default_value='kNearestPoint')])

    NFmiDataIdent.add_copy_constructor()
    NFmiDataIdent.add_constructor([param('const NFmiParam&','theParam'),
                                  param('const NFmiProducer&', 'theProducer',default_value='NFmiProducer(kSynopProducer)'),
				   param('FmiParamType','theType',default_value='kContinuousParam'),
				   param('bool','isGroup',default_value='true'),
				   param('bool','isActive',default_value='true'),
				   param('bool','containsIndividualParams',default_value='true'),
				   param('bool','isDataParam',default_value='true'),    
				   param('bool','hasDataParam',default_value='false'),
				   param('NFmiParamBag*','theSubParamBag',default_value='0',transfer_ownership=False),
				   param('NFmiVoidPtrList*','theSecondaryProducerList',default_value='0', transfer_ownership=False)])

    NFmiParamBag.add_constructor([])

    NFmiStaticTime.add_constructor([])
    NFmiMetTime.add_constructor([])
    NFmiMetTime.add_constructor([param('NFmiMetTime&', 'aMetTime')])
    NFmiMetTime.add_constructor([param('const long', 'datePart'),param('const long','timePart')])
    NFmiMetTime.add_constructor([param('const NFmiMetTime&', 'aMetTime'),param('short','negRange'),param('short','posRange')])
    NFmiMetTime.add_constructor([param('const short','year'),param('const short','month'),param('const short','day')])
    NFmiMetTime.add_constructor([param('const short','year'),
                                 param('const short','month'),
				 param('const short','day'),
				 param('const short','hour'),
				 param('const short','minute',default_value='0'),
				 param('const short','sec',default_value='0'),
				 param('short','timeStep',default_value='60'),
				 param('short','negRange',default_value='0'),
				 param('short','posRange',default_value='0')])    
    
    NFmiTimePerioid.add_constructor([param('long','minutes')])
    NFmiTimePerioid.add_constructor([param('long','years'),param('long','month'),param('long','days'),param('long','hours'),param('long','minutes'),param('long','seconds'),param('long','microseconds',default_value='0')])
  
    NFmiTimeBag.add_constructor([]) 
    NFmiTimeBag.add_constructor([param('const NFmiMetTime&','theFirstTime'),param('const NFmiMetTime&','theLastTime'),param('NFmiTimePerioid','theTimeResolution')])

    NFmiTime.add_copy_constructor()   
    NFmiTime.add_constructor([param('long','datePart'),param('long','timePart')])
    NFmiTime.add_constructor([param('short','year'),param('short','month'),param('short','day')])
    NFmiTime.add_constructor([param('short','year'),param('short','month'),param('short','day'),param('short','hour'),param('short','minute',default_value='0'),param('short','second',default_value='0')])

    NFmiMetTime.add_constructor([param('const long','datePart'),param('const long','timePart')])
    NFmiMetTime.add_constructor([param('const short','year'),param('const short','month'),param('const short','day')])
    NFmiMetTime.add_constructor([param('const short','year'),param('const short','month'),param('const short','day'),param('const short','hour'),param('const short','minute',default_value='0'),param('const short','second',default_value='0'),param('long','timeStep',default_value='60'),param('short','negRange',default_value='0'),param('short','posRange',default_value='0')])

    NFmiProducer.add_constructor([])
    NFmiProducer.add_constructor([param('const NFmiProducer&','theProducer')])
    NFmiProducer.add_constructor([param('unsigned long','theIdent'),param('const NFmiString&','theName',default_value='"Kennel"')])

    NFmiLevel.add_copy_constructor()
    NFmiLevel.add_constructor([])
    NFmiLevel.add_constructor([param('FmiLevelType', 'theLevelType'), param('float', 'theLevelValue')])

    NFmiLocation.add_copy_constructor()
    NFmiLocation.add_constructor([])
    NFmiLocation.add_constructor([param('const NFmiPoint &', 'theLonLatPoint')])

    NFmiEnumConverter.add_constructor([])

    NFmiLevelBag.add_constructor([])

    NFmiGdalArea.add_constructor([])
    NFmiGdalArea.add_constructor([param('const std::string&', 'theDatum'), param('const std::string&', 'theDescription'), param('const NFmiPoint&', 'theBottomLeftLatLon'), param('const NFmiPoint&', 'theTopRightLatLon')])

    NFmiStereographicArea.add_constructor([])
    NFmiStereographicArea.add_constructor([param('const NFmiPoint&', 'theBottomLeftLatLon'), param('const NFmiPoint&', 'theTopRightLatLon'), param('const double', 'theCentralLongitude',default_value='0'),param('const NFmiPoint&', 'thetopLeftXY',default_value='NFmiPoint(0.f, 0.f)'),param('const NFmiPoint&', 'theBottomRightXY',default_value='NFmiPoint(1.f, 1.f)'),param('const double', 'theCentralLatitude',default_value='90'),param('const double', 'theTrueLatitude',default_value='60')])

    # methods

    NFmiHPlaceDescriptor.add_method('IsLocation', retval('bool'), [], is_const=True)
    NFmiHPlaceDescriptor.add_method('IsArea', retval('bool'), [], is_const=True)
    NFmiHPlaceDescriptor.add_method('IsGrid', retval('bool'), [], is_const=True)

    NFmiArea.add_method('BottomLeftLatLon', retval('const NFmiPoint'), [], is_const=True)
    NFmiArea.add_method('BottomRightLatLon', retval('const NFmiPoint'), [], is_const=True)
    NFmiArea.add_method('TopLeftLatLon', retval('const NFmiPoint'), [], is_const=True)
    NFmiArea.add_method('TopRightLatLon', retval('const NFmiPoint'), [], is_const=True)

    NFmiString.add_method('TrimR', None, [param('unsigned char', 'theChar', default_value="' '")])
    NFmiString.add_method('CharPtr', retval('const char *'), [], is_const=True)

    NFmiStreamQueryData.add_method('WriteData', 
                                   retval('bool'), 
				   [param('const NFmiString&','theFileName'),
                                    param('NFmiQueryData*','theQueryData',default_value='0',transfer_ownership=True),
				    param('long','theLibVersion',default_value='6')],
				    is_const=True)
				    
    NFmiStreamQueryData.add_method('WriteCout', retval('bool'), [param('NFmiQueryData*','theQueryData',transfer_ownership=False,default_value='0')],is_const=True)
    NFmiStreamQueryData.add_method('QueryInfoIter', retval('NFmiFastQueryInfo*', caller_owns_return=True), [])
    NFmiStreamQueryData.add_method('ReadIn', retval('bool'), [param('NFmiQueryData*', 'theQueryData', default_value='0', transfer_ownership=False)])
    NFmiStreamQueryData.add_method('QueryData', retval('NFmiQueryData*', reference_existing_object=True), [])


    NFmiIndividual.add_method('GetName', retval('const NFmiString&'), [], is_const=True)
    NFmiIndividual.add_method('GetIdent', retval('unsigned long'), [], is_const=True)
    NFmiIndividual.add_method('SetName', None, [param('const NFmiString&', 'theName')])
    NFmiIndividual.add_method('SetIdent', None, [param('unsigned long', 'theIdent')])

    NFmiStaticTime.add_method('GetYear', 'short', [], is_const=True)
    NFmiStaticTime.add_method('GetMonth', 'short', [], is_const=True)
    NFmiStaticTime.add_method('GetDay', 'short', [], is_const=True)
    NFmiStaticTime.add_method('GetHour', 'short', [], is_const=True)
    NFmiStaticTime.add_method('GetMin', 'short', [], is_const=True)
    NFmiStaticTime.add_method('GetSec', 'short', [], is_const=True)

    NFmiMetTime.add_output_stream_operator()

    NFmiTime.add_method('ToStr', 'NFmiString', [param('const NFmiString', 'theTimeFormat'),param('const FmiLanguage', 'theLanguage', default_value='kFinnish')], is_const=True,is_virtual=True)

    NFmiPoint.add_method('Set', None, [param('double','newX'), param('double', 'newY')])
    NFmiPoint.add_method('X', retval('double'), [], is_const=True)
    NFmiPoint.add_method('Y', retval('double'), [], is_const=True)

    NFmiDataIdent.add_method('GetParam', retval('NFmiParam*', reference_existing_object=True), [], is_const=True)
    NFmiDataIdent.add_method('GetProducer', retval('NFmiProducer*', reference_existing_object=True), [], is_const=True)
    NFmiDataIdent.add_method('SetParam', None, [param('const NFmiParam&', 'theParam')])
    NFmiDataIdent.add_method('SetProducer', None, [param('const NFmiProducer&', 'theProducer')])

    NFmiString.add_output_stream_operator()
    NFmiPoint.add_output_stream_operator()
    NFmiDataIdent.add_output_stream_operator()
    NFmiParamBag.add_output_stream_operator()
    NFmiParamDescriptor.add_output_stream_operator()
    NFmiQueryInfo.add_output_stream_operator()
    NFmiQueryData.add_output_stream_operator()
    NFmiParam.add_output_stream_operator()

    #NFmiSize.add_output_stream_operator()

    NFmiQueryInfo.add_method('SizeParams', retval('unsigned long'), [], is_const=True, is_virtual=True)
    NFmiQueryInfo.add_method('SizeTimes', retval('unsigned long'), [], is_const=True, is_virtual=True)
    NFmiQueryInfo.add_method('SizeLocations', retval('unsigned long'), [], is_const=True, is_virtual=True)
    NFmiQueryInfo.add_method('SizeLevels', retval('unsigned long'), [], is_const=True, is_virtual=True)
    NFmiQueryInfo.add_method('FloatValue', retval('float'), [], is_const=True, is_virtual=True)
    NFmiQueryInfo.add_method('FloatValue', retval('bool'), [param('float','theFloatData')], is_virtual=True)
    NFmiQueryInfo.add_method('InterpolatedValue', retval('float'), [param('const NFmiPoint&', 'theLatLonPoint')], is_virtual=True)
    NFmiQueryInfo.add_method('First', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('FirstParam', retval('bool'), [param('bool', 'fIgnoreSubParam', default_value='true')], is_virtual=True)
    NFmiQueryInfo.add_method('FirstLocation', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('FirstTime', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('FirstLevel', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('NextParam', retval('bool'), [param('bool', 'fIgnoreSubParam', default_value='true')], is_virtual=True)
    NFmiQueryInfo.add_method('PreviousParam', retval('bool'), [param('bool', 'fIgnoreSubParam', default_value='true')], is_virtual=True)
    NFmiQueryInfo.add_method('NextLocation', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('PreviousLocation', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('NextTime', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('PreviousTime', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('NextLevel', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('PreviousLevel', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('Reset', None, [], is_virtual=True)
    NFmiQueryInfo.add_method('ResetParam', None, [], is_virtual=True)
    NFmiQueryInfo.add_method('ResetLocation', None, [], is_virtual=True)
    NFmiQueryInfo.add_method('ResetTime', None, [], is_virtual=True)
    NFmiQueryInfo.add_method('ResetLevel', None, [], is_virtual=True)
    NFmiQueryInfo.add_method('Location', retval('const NFmiLocation*'), [], is_const=True, is_virtual=True)
    NFmiQueryInfo.add_method('ValidTime', retval('const NFmiMetTime&'), [], is_const=True, is_virtual=True)
    NFmiQueryInfo.add_method('Param', retval('NFmiDataIdent&'), [], is_const=True, is_virtual=True)
    NFmiQueryInfo.add_method('IsGrid', retval('bool'), [], is_const=True)
    NFmiQueryInfo.add_method('IsArea', retval('bool'), [], is_const=True)
    NFmiQueryInfo.add_method('IndexFloatValue', retval('float'), [param('unsigned long', 'theIndex')], is_const=True, visibility='protected')
    NFmiQueryInfo.add_method('Index', retval('unsigned long'), [], is_const=True, is_virtual=True, visibility='protected')
    NFmiQueryInfo.add_method('IsSubParamUsed', retval('bool'), [], is_const=True, is_virtual=True, visibility='protected')
    NFmiQueryInfo.add_method('SubParamFloatValue', retval('float'), [], is_const=True, visibility='protected')
    NFmiQueryInfo.add_method('SetProducer',None,[param('const NFmiProducer&','newProducer')])
    NFmiQueryInfo.add_method('Grid',retval('const NFmiGrid*'), [], is_const=True)
    NFmiQueryInfo.add_method('HPlaceDescriptor',retval('const NFmiHPlaceDescriptor&'), [], is_const=True)
    NFmiQueryInfo.add_method('EditLevel',retval('NFmiLevel&'), [], is_const=False)
    NFmiQueryInfo.add_method('EditStation',retval('NFmiStation&'), [], is_const=False)
    NFmiQueryInfo.add_method('EditParam',retval('NFmiDataIdent&'), [], is_const=False)
    NFmiQueryInfo.add_method('SetHPlaceDescriptor', None, [param('const NFmiHPlaceDescriptor&', 'newDesc')])
    NFmiQueryInfo.add_method('SetVPlaceDescriptor', None, [param('const NFmiVPlaceDescriptor&', 'newDesc')])
    NFmiQueryInfo.add_method('SetTimeDescriptor', None, [param('const NFmiTimeDescriptor&', 'newDesc')])
    NFmiQueryInfo.add_method('SetParamDescriptor', None, [param('const NFmiParamDescriptor&', 'newDesc')])
    NFmiQueryInfo.add_method('TimeIndex', retval('unsigned long'), [], is_const=True, is_virtual=True)
    NFmiQueryInfo.add_method('LevelIndex', retval('unsigned long'), [], is_const=True, is_virtual=True)
    NFmiQueryInfo.add_method('ParamIndex', retval('unsigned long'), [], is_const=True, is_virtual=True)
    NFmiQueryInfo.add_method('LocationIndex', retval('unsigned long'), [], is_const=True, is_virtual=True)
    NFmiQueryInfo.add_method('TimeIndex', retval('bool'), [param('unsigned long', 'theIndex')], is_virtual=True)
    NFmiQueryInfo.add_method('LevelIndex', retval('bool'), [param('unsigned long', 'theIndex')], is_virtual=True)
    NFmiQueryInfo.add_method('ParamIndex', retval('bool'), [param('unsigned long', 'theIndex')], is_virtual=True)
    NFmiQueryInfo.add_method('LocationIndex', retval('bool'), [param('unsigned long', 'theIndex')], is_virtual=True)
    NFmiQueryInfo.add_method('HPlaceDescriptor', retval('const NFmiHPlaceDescriptor&'), [], is_const=True)
    NFmiQueryInfo.add_method('VPlaceDescriptor', retval('const NFmiVPlaceDescriptor&'), [], is_const=True)
    NFmiQueryInfo.add_method('TimeDescriptor', retval('const NFmiTimeDescriptor&'), [], is_const=True)
    NFmiQueryInfo.add_method('ParamDescriptor', retval('const NFmiParamDescriptor&'), [], is_const=True)
    NFmiQueryInfo.add_method('Left', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('Right', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('Top', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('Bottom', retval('bool'), [], is_virtual=True)
    NFmiQueryInfo.add_method('Area', retval('const NFmiArea*', reference_existing_object=True), [], is_const=True)
    NFmiQueryInfo.add_method('LatLon', retval('NFmiPoint'), [], is_const=True, is_virtual=True)

    NFmiKKJArea.add_method('LatLonToWorldXY', retval('const NFmiPoint'), [param('const NFmiPoint&', 'theLatLonPoint')], is_const=True, is_virtual=True)
    NFmiKKJArea.add_method('WorldXYToLatLon', retval('const NFmiPoint'), [param('const NFmiPoint&', 'theXYPoint')], is_const=True, is_virtual=True);

    NFmiLatLonArea.add_method('LatLonToWorldXY', retval('const NFmiPoint'), [param('const NFmiPoint&', 'theLatLonPoint')], is_const=True, is_virtual=True)
#    NFmiKKJArea.add_method('LatLonToWorldXY', retval('const NFmiPoint'), [param('const NFmiPoint&', 'theLatLonPoint')], is_const=True, is_virtual=True)

    NFmiGridBase.add_method('Size', retval('unsigned long'), [], is_const=True)
    NFmiGridBase.add_method('Index', retval('unsigned long'), [], is_const=True)
    NFmiGridBase.add_method('XNumber', retval('unsigned long'), [], is_const=True)
    NFmiGridBase.add_method('YNumber', retval('unsigned long'), [], is_const=True)
    NFmiGridBase.add_method('ResetX', None, [])
    NFmiGridBase.add_method('ResetY', None, [])
    NFmiGridBase.add_method('First', retval('bool'), [])
    NFmiGridBase.add_method('Last', retval('bool'), [])
    NFmiGridBase.add_method('Next', retval('bool'), [param('unsigned long','numberOfSteps',default_value="1")], is_virtual=True)
    NFmiGridBase.add_method('Up', retval('bool'), [param('unsigned long','numberOfSteps',default_value="1")], is_virtual=True)
    NFmiGridBase.add_method('Down', retval('bool'), [param('unsigned long','numberOfSteps',default_value="1")], is_virtual=True)
    NFmiGridBase.add_method('Right', retval('bool'), [param('unsigned long','numberOfSteps',default_value="1")], is_virtual=True)
    NFmiGridBase.add_method('Left', retval('bool'), [param('unsigned long','numberOfSteps',default_value="1")], is_virtual=True)    
    NFmiGridBase.add_method('FloatValue', retval('double'), [param('const NFmiPoint&', 'gridPoint')]) 
    NFmiGridBase.add_method('GridPoint', retval('const NFmiPoint'), [], is_const=True, is_virtual=True) 


    NFmiGrid.add_method('WorldXY', retval('const NFmiPoint'), [param('unsigned long', 'theIndex')], is_const=True)
    NFmiGrid.add_method('LatLon', retval('const NFmiPoint'), [param('unsigned long', 'theIndex')], is_const=True)
    NFmiGrid.add_method('LatLon', retval('const NFmiPoint'), [], is_const=True)
    NFmiGrid.add_method('XY', retval('const NFmiPoint'), [], is_const=True)

    NFmiFastQueryInfo.add_method('Param',retval('bool'), [param('const NFmiParam&', 'theParam')])
    NFmiFastQueryInfo.add_method('Param',retval('NFmiDataIdent&'),[], is_const=True)
    NFmiFastQueryInfo.add_method('OriginTime',retval('const NFmiMetTime&'), [], is_const=True)
    NFmiFastQueryInfo.add_method('OriginTime',retval('void'), [param('const NFmiMetTime&', 'newTime')], is_virtual=True)
    NFmiFastQueryInfo.add_method('Time',retval('bool'),[param('const NFmiMetTime&', 'theTime')])
    NFmiFastQueryInfo.add_method('Time',retval('const NFmiMetTime&'),[], is_const=True)
    NFmiFastQueryInfo.add_method('Level',retval('bool'), [param('const NFmiLevel&', 'theLevelValue')])    
    NFmiFastQueryInfo.add_method('Level',retval('const NFmiLevel *'),[], is_const=True)
    NFmiFastQueryInfo.add_method('Location',retval('bool'),[param('const NFmiLocation&', 'theLocation')])
    NFmiFastQueryInfo.add_method('Location',retval('bool'),[param('const unsigned long&', 'theIdent')])
    NFmiFastQueryInfo.add_method('Location',retval('const NFmiLocation *'),[], is_const=True)
    NFmiFastQueryInfo.add_method('Location',retval('bool'),[param('const NFmiPoint&', 'theLonLatPoint'), param('NFmiPoint *','theGridPoint',default_value=0,transfer_ownership=True)])
    NFmiFastQueryInfo.add_method('LastLevel',retval('bool'),[])
    NFmiFastQueryInfo.add_method('LastParam',retval('bool'),[param('bool', 'fIgnoreSubParam', default_value='true')])
    NFmiFastQueryInfo.add_method('LastLocation',retval('bool'),[])
    NFmiFastQueryInfo.add_method('LastTime',retval('bool'),[], is_virtual=True)
    NFmiFastQueryInfo.add_method('MoveRight', retval('bool'), [param('int','moveBy',default_value="1")])
    NFmiFastQueryInfo.add_method('MoveUp', retval('bool'), [param('int','moveBy',default_value="1")])
    NFmiFastQueryInfo.add_method('MoveLeft', retval('bool'), [param('int','moveBy',default_value="1")])                               
    NFmiFastQueryInfo.add_method('MoveDown', retval('bool'), [param('int','moveBy',default_value="1")]) 

    NFmiLocationBag.add_method('AddLocation', retval('bool'), [param('const NFmiLocation&', 'theLocation'),param('bool', 'theChecking', default_value='true')], is_virtual=True)
    NFmiStationBag.add_method('AddLocation', retval('bool'), [param('const NFmiLocation&', 'theLocation'),param('bool', 'theChecking', default_value='true')], is_virtual=True)
    NFmiStationBag.add_method('AddStation', retval('bool'), [param('const NFmiStation&', 'theStation')])

    NFmiSize.add_method('GetSize',retval('unsigned long'),[], is_virtual=True, is_const=True)
    
    NFmiParamBag.add_method('Add',retval('bool'),[param('const NFmiDataIdent&','theParam'),param('bool','fCheckNoDuplicateParams',default_value='false')])
    NFmiParamBag.add_method('Remove',retval('bool'),[])

    NFmiTimeList.add_method('Add',
                            None,
			    [param('NFmiMetTime*', 'theItem',transfer_ownership=True),
			     param('bool','fAllowDuplicates',default_value='false'),
			     param('bool','fAddEnd',default_value='true')],
			    is_virtual=True)
			    
    NFmiTimeList.add_method('Add',None,[param('NFmiTimeList*', 'theList',transfer_ownership=True)], is_virtual=True)
    NFmiTimeList.add_method('Reset',retval('bool'), [], is_const=True)
    NFmiTimeList.add_method('First',retval('bool'), [], is_const=True)
    NFmiTimeList.add_method('Next',retval('bool'), [], is_const=True)
    NFmiTimeList.add_method('Previous',retval('bool'), [], is_const=True)
    NFmiTimeList.add_method('NumberOfItems',retval('int'), [], is_const=True)
    NFmiTimeList.add_method('FirstTime',retval('const NFmiMetTime&'), [], is_const=True)
    NFmiTimeList.add_method('LastTime',retval('const NFmiMetTime&'), [], is_const=True)
    NFmiTimeList.add_method('Find',retval('bool'), [param('const NFmiMetTime&','theTime')], is_const=True)

    NFmiTimeBag.add_method('FirstTime',retval('const NFmiMetTime&'), [], is_const=True)
    NFmiTimeBag.add_method('LastTime',retval('const NFmiMetTime&'), [], is_const=True)
    NFmiTimeBag.add_method('MoveByMinutes',None,[param('long', 'minutes')])

    NFmiQueryDataUtil.add_method('CreateEmptyData',retval('NFmiQueryData*',caller_owns_return=True),[param('NFmiQueryInfo&','srcInfo')], is_static=True)
        
    NFmiQueryData.add_method('Info',retval('NFmiQueryInfo*',caller_owns_return=True),[], is_const=True)
    NFmiQueryData.add_method('Write',None,[param('const std::string&', 'filename'), param('bool', 'forceBinaryFormat', default_value='true')], is_const=True)
  
    NFmiTimeDescriptor.add_method('OriginTime',retval('const NFmiMetTime&'), [], is_const=True)
    NFmiTimeDescriptor.add_method('ValidTimeBag',retval('NFmiTimeBag *', reference_existing_object=True), [])
    
    NFmiLocation.add_method('GetLocation', retval('const NFmiPoint'), [], is_const=True)

    NFmiMetTime.add_method('IsEqual',retval('bool'), [param('const NFmiSortable&', 'aFmiTest')], is_const=True, is_virtual=True)

    NFmiTime.add_method('ChangeBySeconds', None, [param('long', 'seconds')])
    NFmiTime.add_method('ChangeByMinutes', None, [param('long', 'minutes')])
    NFmiTime.add_method('ChangeByHours', None, [param('long', 'hours')])
    NFmiTime.add_method('ChangeByDays', None, [param('long', 'days')])
    NFmiTime.add_method('DifferenceInMinutes', retval('long'), [param('const NFmiTime &', 'anotherTime')], is_const=True)
    NFmiTime.add_method('DifferenceInHours', retval('long'), [param('const NFmiTime &', 'anotherTime')], is_const=True)
    NFmiTime.add_method('DifferenceInDays', retval('long'), [param('const NFmiTime &', 'anotherTime')], is_const=True)
    NFmiTime.add_method('UTCTime', retval('const NFmiTime'), [param('float', 'theLongitude', default_value='32700')], is_const=True, is_virtual=True)

    NFmiLevel.add_method('LevelValue', retval('float'), [], is_const=True)
    NFmiLevel.add_method('LevelType', retval('FmiLevelType'), [], is_const=True)

    NFmiEnumConverter.add_method('ToString', retval('const std::string'), [param('int', 'theValue')])
    NFmiEnumConverter.add_method('ToEnum', retval('int'), [param('const char* ', 's')])

    NFmiLevelBag.add_method('AddLevel', retval('bool'), [param('const NFmiLevel&', 'theLevel')])
    NFmiLevelBag.add_method('Level', retval('NFmiLevel*', reference_existing_object=True), [], is_const=True)
    NFmiLevelBag.add_method('Level', retval('NFmiLevel*', reference_existing_object=True), [param('unsigned long', 'theIndex')], is_const=True)

    NFmiGdalArea.add_method('WKT', retval('const std::string'), [], is_virtual=True, is_const=True)
    NFmiGdalArea.add_method('LatLonToWorldXY', retval('const NFmiPoint'), [param('const NFmiPoint&', 'theLatLonPoint')], is_const=True, is_virtual=True)
    NFmiGdalArea.add_method('WorldXYToLatLon', retval('const NFmiPoint'), [param('const NFmiPoint&', 'theXYPoint')], is_const=True, is_virtual=True);
    NFmiGdalArea.add_method('ToXY', retval('NFmiPoint'), [param('const NFmiPoint&', 'theLatLonPoint')], is_const=True, is_virtual=True);

def includeEnums(mod):

    print("//==================== Enums ====================")

    directions = extract_enum_from_header('../newbase/NFmiGlobals.h', 'FmiDirection')
    print("// {} members of FmiDirection extracted from NFmiGlobals.h".format(len(directions)))
    mod.add_enum('FmiDirection', directions)

    mod.add_enum('kFloatMissing', [('kFloatMissing', 32700)]) # Fake enum :(

    level_types = extract_enum_from_header('../newbase/NFmiLevelType.h', 'FmiLevelType')
    print("// {} members of FmiLevelType extracted from NLevelType.h".format(len(level_types)))
    mod.add_enum('FmiLevelType', level_types)

    interpolation_methods = extract_enum_from_header('../newbase/NFmiGlobals.h', 'FmiInterpolationMethod')
    print("// {} members of FmiInterpolationMethod extracted from NFmiGlobals.h".format(len(interpolation_methods)))
    mod.add_enum('FmiInterpolationMethod', ['kNoneInterpolation', 'kLinearly', 'kNearestPoint', 'kByCombinedParam', 'kLinearlyFast', 'kLagrange'])
    
    languages = extract_enum_from_header('../newbase/NFmiGlobals.h', 'FmiLanguage')
    print("// {} members of FmiLanguage extracted from NFmiGlobals.h".format(len(languages)))
    mod.add_enum('FmiLanguage', ['kFinnish', 'kSwedish', 'kEnglish', 'kSpanish', 'kGermany', 'kFrench', 'kSaame', 'kLatvian', 'kChinese'])

    station_types = extract_enum_from_header('../newbase/NFmiGlobals.h', 'FmiStationType')
    print("// {} members of FmiStationType extracted from NFmiGlobals.h".format(len(station_types)))
    mod.add_enum('FmiStationType', station_types)
    
    param_types = extract_enum_from_header('../newbase/NFmiDataIdent.h', 'FmiParamType')
    print("// {} members of FmiParamType extracted from NFmiDataIdent.h".format(len(param_types)))
    mod.add_enum('FmiParamType', param_types)
		 
    param_names_members = extract_enum_from_header('../newbase/NFmiParameterName.h', 'FmiParameterName', 'kFmi')
    print("// {} members of FmiParameterName extracted from NFmiParameterName.h".format(len(param_names_members)))
    mod.add_enum('FmiParameterName', param_names_members)

    mod.add_enum('FmiTimeLocalzation',['kUTC','kLongitude','kState'])
    print("//==================== End of Enums ==============")

def generateCode():

    mod = Module(MODULE_NAME)

    addIncludes(mod)

    includeEnums(mod) 
    includeClasses(mod)

    mod.generate(sys.stdout)


if __name__ == "__main__":
    generateCode()

