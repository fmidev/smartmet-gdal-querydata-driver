/*!
 *  \file
 *
 *  NFmiWindFix interface is meant to fix main level wind parameters after interpolations.
 *  Problem: When main level u- and v -wind components are interpolated
 *  it's done without the whole wind-vector knowledge. So if e.g. u -component
 *  turns 180 degrees between interpolated time steps and first value is 10.4 m/s
 *  and second value is -10.2 m/s, interpolation results:
 *  factor * 10.4 + (1 - factor)*(-10.2) and if factor is 0.5 => u ~ 0 m/s
 *  So this way of interpolation can only reduce wind speeds because calculations are
 *  made only using single components.
 *  NFmiWindFix interface offers two functions that are meant to be used in any
 *  interpolation qd-filter programs (e.g. qdinterpolatetime). You call these
 *  functions after normal interpolation is done.
 *  In order anything to happen in these fix-functions, queryData needs to have
 *  WD and WD parameters in main level, and also some of or all of u, v and
 *  wind-vector parameters.
 *  If queryData has TotalWind, nothing will be done.
 *
 */

class NFmiQueryData;
class NFmiFastQueryInfo;

namespace NFmiWindFix
{
bool FixWinds(NFmiQueryData& sourceData);
bool FixWinds(NFmiFastQueryInfo& sourceInfo);

}  // namespace NFmiWindFix
