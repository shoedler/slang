print Float.nan     // [expect] nan
print Float.nnan    // [expect] -nan
print Float.inf     // [expect] inf
print Float.ninf    // [expect] -inf

print Float.nan == Float.nan   // [expect] false
print Float.nnan == Float.nnan // [expect] false

print Float.max // [expect] 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878
print Float.min // [expect] -17976931348623157081452742373170435679807056752584499659891747680315726078002853876058955863276687

print Int.max // [expect] 9223372036854775808
print Int.min // [expect] -9223372036854775808 
