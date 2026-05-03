import newbase
from newbase import *
from datetime import datetime,timedelta
import json

def getMatchingId(coordDict,lat,lon):
    latlon_dir=["%7.4f" % coordDict['lat'][i]+" "+"%7.4f" % coordDict['lon'][i] \
               for i in range(0,len(coordDict['lat']))]
    try:
       idx=latlon_dir.index(lat+" "+lon)
    except:
       return "-99"
    return coordDict['statId'][idx]

def readQueryData(queryData,coordDict,Variables):
    variable_names=[var[0] for var in Variables]
    selected_parameters=[var[1] for var in Variables]
    decimals=[var[2] for var in Variables]
    #read querydata
    fqd = NFmiQueryData(queryData)
    fqi = NFmiFastQueryInfo(fqd)
    
    fqi.Reset()
    fqi.FirstLevel()
    
    data=[] 
    #go trough locations
    while fqi.NextLocation():
        fqi.ResetTime() #reset time before next parameter 		
        pointID ="%06d" % fqi.Location().GetIdent() #get point id
        
        gp=fqi.Location().GetLocation()
        lat="%7.4f" % gp.Y()
        lon="%7.4f" % gp.X()
        lat=lat.strip()
        lon=lon.strip()
        id=getMatchingId(coordDict,lat,lon)
        if id=="-99":
           continue
        idx=coordDict["statId"].index(id)
        lat=("%10.7f" % coordDict["lat"][idx]).strip()
        lon=("%10.7f" % coordDict["lon"][idx]).strip()
        #go trough times
        
        while fqi.NextTime():
           fqi.ResetParam()
           #time to string
           date = "%s" % fqi.ValidTime().ToStr(NFmiString('YYYYMMDDhhmm'))
           date=datetime.strptime(date.strip().split()[1],"%Y%m%d%H%M")
           date=date.strftime("%Y%m%dT%H%M%S")
           
           entry = {
              "utctime": date,
              "latitude": lat,
              "longitude": lon
           }
            #go trough parameters in querydata
           while fqi.NextParam():
    
                #get parameter id
                parameterID="%s" % fqi.Param().GetParam().GetIdent()
                #if the parameter is not in selected parameters, skip it
                if parameterID not in selected_parameters:
                    continue 
                
                value = fqi.FloatValue() #get parameter value
    
                idx_par=selected_parameters.index(parameterID)
                N_decimals=decimals[idx_par]

                #if value is missing set to NaN
                if value == kFloatMissing:
                   val_string ='NaN'
                else:
                   if N_decimals==0:
                       val_string=str(int(value))
                   else:
                       f="%15."+str(N_decimals)+"f"
                       val_string= f % value
                       val_string=val_string.strip()
                entry[variable_names[idx_par]]=(val_string)
           data.append(entry)
               
    return data

# Example usage
Variables=[ \
["Temperature", "4", 2]]
coordDict={"statId":[0000],"lon":[26.6136],"lat":[67.3928],"name":["point0000"]}
forecastFileName="test_data.sqd.xz"
# Forecast data is saved in nested dictionary. 
# First level is point id and third the variable number ("time" for timestamp)
# For example, to get surface tempreature forecast  for point 0001 use 
# data['10001']['648'] 
forecast_data=readQueryData(forecastFileName,coordDict,Variables)

if len(forecast_data)>0:
   print("forecast data succesfully read")
   exit(0)
else:
   print("Error, forecast data not read")
   exit(1)
#print(forecast_data)
#outFileName='test_data.json'         
#with open(outFileName,'w') as outfile:
#    json.dump(forecast_data,outfile,indent=2)
