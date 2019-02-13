#include <vector>
#include <Arduino.h>
#include "Config.h"
#include "HttpServer.h"

#include "AutoShutdown.h"
#include <EEPROMInterface.h>
#include <httpd.h>
#include <mico.h>
#include <EMW10xxInterface.h>
#include <SystemVariables.h>
#include <parson.h>
#include <OTAFirmwareUpdate.h>
#include <AZ3166WiFi.h>
#include <ReButton.h>
#include "Display.h"

#define POWER_OFF_TIME	(1000)	// [msec.]

#define app_httpd_log(...)

#define HTTPD_HDR_DEFORT (HTTPD_HDR_ADD_SERVER|HTTPD_HDR_ADD_CONN_CLOSE|HTTPD_HDR_ADD_PRAGMA_NO_CACHE)

extern NetworkInterface *network;

static const char* CSS_BOOTSTRAP_GRID = \
	"/*!\n" \
	" * Bootstrap Grid v4.0.0 (https://getbootstrap.com)\n" \
	" * Copyright 2011-2018 The Bootstrap Authors\n" \
	" * Copyright 2011-2018 Twitter, Inc.\n" \
	" * Licensed under MIT (https://github.com/twbs/bootstrap/blob/master/LICENSE)\n" \
	" */@-ms-viewport{width:device-width}html{box-sizing:border-box;-ms-overflow-style:scrollbar}*,::after,::before{box-sizing:inherit}.container{width:100%;padding-right:15px;padding-left:15px;margin-right:auto;margin-left:auto}@media (min-width:576px){.container{max-width:540px}}@media (min-width:768px){.container{max-width:720px}}@media (min-width:992px){.container{max-width:960px}}@media (min-width:1200px){.container{max-width:1140px}}.container-fluid{width:100%;padding-right:15px;padding-left:15px;margin-right:auto;margin-left:auto}.row{display:-webkit-box;display:-ms-flexbox;display:flex;-ms-flex-wrap:wrap;flex-wrap:wrap;margin-right:-15px;margin-left:-15px}.no-gutters{margin-right:0;margin-left:0}.no-gutters>.col,.no-gutters>[class*=col-]{padding-right:0;padding-left:0}.col,.col-1,.col-10,.col-11,.col-12,.col-2,.col-3,.col-4,.col-5,.col-6,.col-7,.col-8,.col-9,.col-auto,.col-lg,.col-lg-1,.col-lg-10,.col-lg-11,.col-lg-12,.col-lg-2,.col-lg-3,.col-lg-4,.col-lg-5,.col-lg-6,.col-lg-7,.col-lg-8,.col-lg-9,.col-lg-auto,.col-md,.col-md-1,.col-md-10,.col-md-11,.col-md-12,.col-md-2,.col-md-3,.col-md-4,.col-md-5,.col-md-6,.col-md-7,.col-md-8,.col-md-9,.col-md-auto,.col-sm,.col-sm-1,.col-sm-10,.col-sm-11,.col-sm-12,.col-sm-2,.col-sm-3,.col-sm-4,.col-sm-5,.col-sm-6,.col-sm-7,.col-sm-8,.col-sm-9,.col-sm-auto,.col-xl,.col-xl-1,.col-xl-10,.col-xl-11,.col-xl-12,.col-xl-2,.col-xl-3,.col-xl-4,.col-xl-5,.col-xl-6,.col-xl-7,.col-xl-8,.col-xl-9,.col-xl-auto{position:relative;width:100%;min-height:1px;padding-right:15px;padding-left:15px}.col{-ms-flex-preferred-size:0;flex-basis:0;-webkit-box-flex:1;-ms-flex-positive:1;flex-grow:1;max-width:100%}.col-auto{-webkit-box-flex:0;-ms-flex:0 0 auto;flex:0 0 auto;width:auto;max-width:none}.col-1{-webkit-box-flex:0;-ms-flex:0 0 8.333333%;flex:0 0 8.333333%;max-width:8.333333%}.col-2{-webkit-box-flex:0;-ms-flex:0 0 16.666667%;flex:0 0 16.666667%;max-width:16.666667%}.col-3{-webkit-box-flex:0;-ms-flex:0 0 25%;flex:0 0 25%;max-width:25%}.col-4{-webkit-box-flex:0;-ms-flex:0 0 33.333333%;flex:0 0 33.333333%;max-width:33.333333%}.col-5{-webkit-box-flex:0;-ms-flex:0 0 41.666667%;flex:0 0 41.666667%;max-width:41.666667%}.col-6{-webkit-box-flex:0;-ms-flex:0 0 50%;flex:0 0 50%;max-width:50%}.col-7{-webkit-box-flex:0;-ms-flex:0 0 58.333333%;flex:0 0 58.333333%;max-width:58.333333%}.col-8{-webkit-box-flex:0;-ms-flex:0 0 66.666667%;flex:0 0 66.666667%;max-width:66.666667%}.col-9{-webkit-box-flex:0;-ms-flex:0 0 75%;flex:0 0 75%;max-width:75%}.col-10{-webkit-box-flex:0;-ms-flex:0 0 83.333333%;flex:0 0 83.333333%;max-width:83.333333%}.col-11{-webkit-box-flex:0;-ms-flex:0 0 91.666667%;flex:0 0 91.666667%;max-width:91.666667%}.col-12{-webkit-box-flex:0;-ms-flex:0 0 100%;flex:0 0 100%;max-width:100%}.order-first{-webkit-box-ordinal-group:0;-ms-flex-order:-1;order:-1}.order-last{-webkit-box-ordinal-group:14;-ms-flex-order:13;order:13}.order-0{-webkit-box-ordinal-group:1;-ms-flex-order:0;order:0}.order-1{-webkit-box-ordinal-group:2;-ms-flex-order:1;order:1}.order-2{-webkit-box-ordinal-group:3;-ms-flex-order:2;order:2}.order-3{-webkit-box-ordinal-group:4;-ms-flex-order:3;order:3}.order-4{-webkit-box-ordinal-group:5;-ms-flex-order:4;order:4}.order-5{-webkit-box-ordinal-group:6;-ms-flex-order:5;order:5}.order-6{-webkit-box-ordinal-group:7;-ms-flex-order:6;order:6}.order-7{-webkit-box-ordinal-group:8;-ms-flex-order:7;order:7}.order-8{-webkit-box-ordinal-group:9;-ms-flex-order:8;order:8}.order-9{-webkit-box-ordinal-group:10;-ms-flex-order:9;order:9}.order-10{-webkit-box-ordinal-group:11;-ms-flex-order:10;order:10}.order-11{-webkit-box-ordinal-group:12;-ms-flex-order:11;order:11}.order-12{-webkit-box-ordinal-group:13;-ms-flex-order:12;order:12}.offset-1{margin-left:8.333333%}.offset-2{margin-left:16.666667%}.offset-3{margin-left:25%}.offset-4{margin-left:33.333333%}.offset-5{margin-left:41.666667%}.offset-6{margin-left:50%}.offset-7{margin-left:58.333333%}.offset-8{margin-left:66.666667%}.offset-9{margin-left:75%}.offset-10{margin-left:83.333333%}.offset-11{margin-left:91.666667%}@media (min-width:576px){.col-sm{-ms-flex-preferred-size:0;flex-basis:0;-webkit-box-flex:1;-ms-flex-positive:1;flex-grow:1;max-width:100%}.col-sm-auto{-webkit-box-flex:0;-ms-flex:0 0 auto;flex:0 0 auto;width:auto;max-width:none}.col-sm-1{-webkit-box-flex:0;-ms-flex:0 0 8.333333%;flex:0 0 8.333333%;max-width:8.333333%}.col-sm-2{-webkit-box-flex:0;-ms-flex:0 0 16.666667%;flex:0 0 16.666667%;max-width:16.666667%}.col-sm-3{-webkit-box-flex:0;-ms-flex:0 0 25%;flex:0 0 25%;max-width:25%}.col-sm-4{-webkit-box-flex:0;-ms-flex:0 0 33.333333%;flex:0 0 33.333333%;max-width:33.333333%}.col-sm-5{-webkit-box-flex:0;-ms-flex:0 0 41.666667%;flex:0 0 41.666667%;max-width:41.666667%}.col-sm-6{-webkit-box-flex:0;-ms-flex:0 0 50%;flex:0 0 50%;max-width:50%}.col-sm-7{-webkit-box-flex:0;-ms-flex:0 0 58.333333%;flex:0 0 58.333333%;max-width:58.333333%}.col-sm-8{-webkit-box-flex:0;-ms-flex:0 0 66.666667%;flex:0 0 66.666667%;max-width:66.666667%}.col-sm-9{-webkit-box-flex:0;-ms-flex:0 0 75%;flex:0 0 75%;max-width:75%}.col-sm-10{-webkit-box-flex:0;-ms-flex:0 0 83.333333%;flex:0 0 83.333333%;max-width:83.333333%}.col-sm-11{-webkit-box-flex:0;-ms-flex:0 0 91.666667%;flex:0 0 91.666667%;max-width:91.666667%}.col-sm-12{-webkit-box-flex:0;-ms-flex:0 0 100%;flex:0 0 100%;max-width:100%}.order-sm-first{-webkit-box-ordinal-group:0;-ms-flex-order:-1;order:-1}.order-sm-last{-webkit-box-ordinal-group:14;-ms-flex-order:13;order:13}.order-sm-0{-webkit-box-ordinal-group:1;-ms-flex-order:0;order:0}.order-sm-1{-webkit-box-ordinal-group:2;-ms-flex-order:1;order:1}.order-sm-2{-webkit-box-ordinal-group:3;-ms-flex-order:2;order:2}.order-sm-3{-webkit-box-ordinal-group:4;-ms-flex-order:3;order:3}.order-sm-4{-webkit-box-ordinal-group:5;-ms-flex-order:4;order:4}.order-sm-5{-webkit-box-ordinal-group:6;-ms-flex-order:5;order:5}.order-sm-6{-webkit-box-ordinal-group:7;-ms-flex-order:6;order:6}.order-sm-7{-webkit-box-ordinal-group:8;-ms-flex-order:7;order:7}.order-sm-8{-webkit-box-ordinal-group:9;-ms-flex-order:8;order:8}.order-sm-9{-webkit-box-ordinal-group:10;-ms-flex-order:9;order:9}.order-sm-10{-webkit-box-ordinal-group:11;-ms-flex-order:10;order:10}.order-sm-11{-webkit-box-ordinal-group:12;-ms-flex-order:11;order:11}.order-sm-12{-webkit-box-ordinal-group:13;-ms-flex-order:12;order:12}.offset-sm-0{margin-left:0}.offset-sm-1{margin-left:8.333333%}.offset-sm-2{margin-left:16.666667%}.offset-sm-3{margin-left:25%}.offset-sm-4{margin-left:33.333333%}.offset-sm-5{margin-left:41.666667%}.offset-sm-6{margin-left:50%}.offset-sm-7{margin-left:58.333333%}.offset-sm-8{margin-left:66.666667%}.offset-sm-9{margin-left:75%}.offset-sm-10{margin-left:83.333333%}.offset-sm-11{margin-left:91.666667%}}@media (min-width:768px){.col-md{-ms-flex-preferred-size:0;flex-basis:0;-webkit-box-flex:1;-ms-flex-positive:1;flex-grow:1;max-width:100%}.col-md-auto{-webkit-box-flex:0;-ms-flex:0 0 auto;flex:0 0 auto;width:auto;max-width:none}.col-md-1{-webkit-box-flex:0;-ms-flex:0 0 8.333333%;flex:0 0 8.333333%;max-width:8.333333%}.col-md-2{-webkit-box-flex:0;-ms-flex:0 0 16.666667%;flex:0 0 16.666667%;max-width:16.666667%}.col-md-3{-webkit-box-flex:0;-ms-flex:0 0 25%;flex:0 0 25%;max-width:25%}.col-md-4{-webkit-box-flex:0;-ms-flex:0 0 33.333333%;flex:0 0 33.333333%;max-width:33.333333%}.col-md-5{-webkit-box-flex:0;-ms-flex:0 0 41.666667%;flex:0 0 41.666667%;max-width:41.666667%}.col-md-6{-webkit-box-flex:0;-ms-flex:0 0 50%;flex:0 0 50%;max-width:50%}.col-md-7{-webkit-box-flex:0;-ms-flex:0 0 58.333333%;flex:0 0 58.333333%;max-width:58.333333%}.col-md-8{-webkit-box-flex:0;-ms-flex:0 0 66.666667%;flex:0 0 66.666667%;max-width:66.666667%}.col-md-9{-webkit-box-flex:0;-ms-flex:0 0 75%;flex:0 0 75%;max-width:75%}.col-md-10{-webkit-box-flex:0;-ms-flex:0 0 83.333333%;flex:0 0 83.333333%;max-width:83.333333%}.col-md-11{-webkit-box-flex:0;-ms-flex:0 0 91.666667%;flex:0 0 91.666667%;max-width:91.666667%}.col-md-12{-webkit-box-flex:0;-ms-flex:0 0 100%;flex:0 0 100%;max-width:100%}.order-md-first{-webkit-box-ordinal-group:0;-ms-flex-order:-1;order:-1}.order-md-last{-webkit-box-ordinal-group:14;-ms-flex-order:13;order:13}.order-md-0{-webkit-box-ordinal-group:1;-ms-flex-order:0;order:0}.order-md-1{-webkit-box-ordinal-group:2;-ms-flex-order:1;order:1}.order-md-2{-webkit-box-ordinal-group:3;-ms-flex-order:2;order:2}.order-md-3{-webkit-box-ordinal-group:4;-ms-flex-order:3;order:3}.order-md-4{-webkit-box-ordinal-group:5;-ms-flex-order:4;order:4}.order-md-5{-webkit-box-ordinal-group:6;-ms-flex-order:5;order:5}.order-md-6{-webkit-box-ordinal-group:7;-ms-flex-order:6;order:6}.order-md-7{-webkit-box-ordinal-group:8;-ms-flex-order:7;order:7}.order-md-8{-webkit-box-ordinal-group:9;-ms-flex-order:8;order:8}.order-md-9{-webkit-box-ordinal-group:10;-ms-flex-order:9;order:9}.order-md-10{-webkit-box-ordinal-group:11;-ms-flex-order:10;order:10}.order-md-11{-webkit-box-ordinal-group:12;-ms-flex-order:11;order:11}.order-md-12{-webkit-box-ordinal-group:13;-ms-flex-order:12;order:12}.offset-md-0{margin-left:0}.offset-md-1{margin-left:8.333333%}.offset-md-2{margin-left:16.666667%}.offset-md-3{margin-left:25%}.offset-md-4{margin-left:33.333333%}.offset-md-5{margin-left:41.666667%}.offset-md-6{margin-left:50%}.offset-md-7{margin-left:58.333333%}.offset-md-8{margin-left:66.666667%}.offset-md-9{margin-left:75%}.offset-md-10{margin-left:83.333333%}.offset-md-11{margin-left:91.666667%}}@media (min-width:992px){.col-lg{-ms-flex-preferred-size:0;flex-basis:0;-webkit-box-flex:1;-ms-flex-positive:1;flex-grow:1;max-width:100%}.col-lg-auto{-webkit-box-flex:0;-ms-flex:0 0 auto;flex:0 0 auto;width:auto;max-width:none}.col-lg-1{-webkit-box-flex:0;-ms-flex:0 0 8.333333%;flex:0 0 8.333333%;max-width:8.333333%}.col-lg-2{-webkit-box-flex:0;-ms-flex:0 0 16.666667%;flex:0 0 16.666667%;max-width:16.666667%}.col-lg-3{-webkit-box-flex:0;-ms-flex:0 0 25%;flex:0 0 25%;max-width:25%}.col-lg-4{-webkit-box-flex:0;-ms-flex:0 0 33.333333%;flex:0 0 33.333333%;max-width:33.333333%}.col-lg-5{-webkit-box-flex:0;-ms-flex:0 0 41.666667%;flex:0 0 41.666667%;max-width:41.666667%}.col-lg-6{-webkit-box-flex:0;-ms-flex:0 0 50%;flex:0 0 50%;max-width:50%}.col-lg-7{-webkit-box-flex:0;-ms-flex:0 0 58.333333%;flex:0 0 58.333333%;max-width:58.333333%}.col-lg-8{-webkit-box-flex:0;-ms-flex:0 0 66.666667%;flex:0 0 66.666667%;max-width:66.666667%}.col-lg-9{-webkit-box-flex:0;-ms-flex:0 0 75%;flex:0 0 75%;max-width:75%}.col-lg-10{-webkit-box-flex:0;-ms-flex:0 0 83.333333%;flex:0 0 83.333333%;max-width:83.333333%}.col-lg-11{-webkit-box-flex:0;-ms-flex:0 0 91.666667%;flex:0 0 91.666667%;max-width:91.666667%}.col-lg-12{-webkit-box-flex:0;-ms-flex:0 0 100%;flex:0 0 100%;max-width:100%}.order-lg-first{-webkit-box-ordinal-group:0;-ms-flex-order:-1;order:-1}.order-lg-last{-webkit-box-ordinal-group:14;-ms-flex-order:13;order:13}.order-lg-0{-webkit-box-ordinal-group:1;-ms-flex-order:0;order:0}.order-lg-1{-webkit-box-ordinal-group:2;-ms-flex-order:1;order:1}.order-lg-2{-webkit-box-ordinal-group:3;-ms-flex-order:2;order:2}.order-lg-3{-webkit-box-ordinal-group:4;-ms-flex-order:3;order:3}.order-lg-4{-webkit-box-ordinal-group:5;-ms-flex-order:4;order:4}.order-lg-5{-webkit-box-ordinal-group:6;-ms-flex-order:5;order:5}.order-lg-6{-webkit-box-ordinal-group:7;-ms-flex-order:6;order:6}.order-lg-7{-webkit-box-ordinal-group:8;-ms-flex-order:7;order:7}.order-lg-8{-webkit-box-ordinal-group:9;-ms-flex-order:8;order:8}.order-lg-9{-webkit-box-ordinal-group:10;-ms-flex-order:9;order:9}.order-lg-10{-webkit-box-ordinal-group:11;-ms-flex-order:10;order:10}.order-lg-11{-webkit-box-ordinal-group:12;-ms-flex-order:11;order:11}.order-lg-12{-webkit-box-ordinal-group:13;-ms-flex-order:12;order:12}.offset-lg-0{margin-left:0}.offset-lg-1{margin-left:8.333333%}.offset-lg-2{margin-left:16.666667%}.offset-lg-3{margin-left:25%}.offset-lg-4{margin-left:33.333333%}.offset-lg-5{margin-left:41.666667%}.offset-lg-6{margin-left:50%}.offset-lg-7{margin-left:58.333333%}.offset-lg-8{margin-left:66.666667%}.offset-lg-9{margin-left:75%}.offset-lg-10{margin-left:83.333333%}.offset-lg-11{margin-left:91.666667%}}@media (min-width:1200px){.col-xl{-ms-flex-preferred-size:0;flex-basis:0;-webkit-box-flex:1;-ms-flex-positive:1;flex-grow:1;max-width:100%}.col-xl-auto{-webkit-box-flex:0;-ms-flex:0 0 auto;flex:0 0 auto;width:auto;max-width:none}.col-xl-1{-webkit-box-flex:0;-ms-flex:0 0 8.333333%;flex:0 0 8.333333%;max-width:8.333333%}.col-xl-2{-webkit-box-flex:0;-ms-flex:0 0 16.666667%;flex:0 0 16.666667%;max-width:16.666667%}.col-xl-3{-webkit-box-flex:0;-ms-flex:0 0 25%;flex:0 0 25%;max-width:25%}.col-xl-4{-webkit-box-flex:0;-ms-flex:0 0 33.333333%;flex:0 0 33.333333%;max-width:33.333333%}.col-xl-5{-webkit-box-flex:0;-ms-flex:0 0 41.666667%;flex:0 0 41.666667%;max-width:41.666667%}.col-xl-6{-webkit-box-flex:0;-ms-flex:0 0 50%;flex:0 0 50%;max-width:50%}.col-xl-7{-webkit-box-flex:0;-ms-flex:0 0 58.333333%;flex:0 0 58.333333%;max-width:58.333333%}.col-xl-8{-webkit-box-flex:0;-ms-flex:0 0 66.666667%;flex:0 0 66.666667%;max-width:66.666667%}.col-xl-9{-webkit-box-flex:0;-ms-flex:0 0 75%;flex:0 0 75%;max-width:75%}.col-xl-10{-webkit-box-flex:0;-ms-flex:0 0 83.333333%;flex:0 0 83.333333%;max-width:83.333333%}.col-xl-11{-webkit-box-flex:0;-ms-flex:0 0 91.666667%;flex:0 0 91.666667%;max-width:91.666667%}.col-xl-12{-webkit-box-flex:0;-ms-flex:0 0 100%;flex:0 0 100%;max-width:100%}.order-xl-first{-webkit-box-ordinal-group:0;-ms-flex-order:-1;order:-1}.order-xl-last{-webkit-box-ordinal-group:14;-ms-flex-order:13;order:13}.order-xl-0{-webkit-box-ordinal-group:1;-ms-flex-order:0;order:0}.order-xl-1{-webkit-box-ordinal-group:2;-ms-flex-order:1;order:1}.order-xl-2{-webkit-box-ordinal-group:3;-ms-flex-order:2;order:2}.order-xl-3{-webkit-box-ordinal-group:4;-ms-flex-order:3;order:3}.order-xl-4{-webkit-box-ordinal-group:5;-ms-flex-order:4;order:4}.order-xl-5{-webkit-box-ordinal-group:6;-ms-flex-order:5;order:5}.order-xl-6{-webkit-box-ordinal-group:7;-ms-flex-order:6;order:6}.order-xl-7{-webkit-box-ordinal-group:8;-ms-flex-order:7;order:7}.order-xl-8{-webkit-box-ordinal-group:9;-ms-flex-order:8;order:8}.order-xl-9{-webkit-box-ordinal-group:10;-ms-flex-order:9;order:9}.order-xl-10{-webkit-box-ordinal-group:11;-ms-flex-order:10;order:10}.order-xl-11{-webkit-box-ordinal-group:12;-ms-flex-order:11;order:11}.order-xl-12{-webkit-box-ordinal-group:13;-ms-flex-order:12;order:12}.offset-xl-0{margin-left:0}.offset-xl-1{margin-left:8.333333%}.offset-xl-2{margin-left:16.666667%}.offset-xl-3{margin-left:25%}.offset-xl-4{margin-left:33.333333%}.offset-xl-5{margin-left:41.666667%}.offset-xl-6{margin-left:50%}.offset-xl-7{margin-left:58.333333%}.offset-xl-8{margin-left:66.666667%}.offset-xl-9{margin-left:75%}.offset-xl-10{margin-left:83.333333%}.offset-xl-11{margin-left:91.666667%}}.d-none{display:none!important}.d-inline{display:inline!important}.d-inline-block{display:inline-block!important}.d-block{display:block!important}.d-table{display:table!important}.d-table-row{display:table-row!important}.d-table-cell{display:table-cell!important}.d-flex{display:-webkit-box!important;display:-ms-flexbox!important;display:flex!important}.d-inline-flex{display:-webkit-inline-box!important;display:-ms-inline-flexbox!important;display:inline-flex!important}@media (min-width:576px){.d-sm-none{display:none!important}.d-sm-inline{display:inline!important}.d-sm-inline-block{display:inline-block!important}.d-sm-block{display:block!important}.d-sm-table{display:table!important}.d-sm-table-row{display:table-row!important}.d-sm-table-cell{display:table-cell!important}.d-sm-flex{display:-webkit-box!important;display:-ms-flexbox!important;display:flex!important}.d-sm-inline-flex{display:-webkit-inline-box!important;display:-ms-inline-flexbox!important;display:inline-flex!important}}@media (min-width:768px){.d-md-none{display:none!important}.d-md-inline{display:inline!important}.d-md-inline-block{display:inline-block!important}.d-md-block{display:block!important}.d-md-table{display:table!important}.d-md-table-row{display:table-row!important}.d-md-table-cell{display:table-cell!important}.d-md-flex{display:-webkit-box!important;display:-ms-flexbox!important;display:flex!important}.d-md-inline-flex{display:-webkit-inline-box!important;display:-ms-inline-flexbox!important;display:inline-flex!important}}@media (min-width:992px){.d-lg-none{display:none!important}.d-lg-inline{display:inline!important}.d-lg-inline-block{display:inline-block!important}.d-lg-block{display:block!important}.d-lg-table{display:table!important}.d-lg-table-row{display:table-row!important}.d-lg-table-cell{display:table-cell!important}.d-lg-flex{display:-webkit-box!important;display:-ms-flexbox!important;display:flex!important}.d-lg-inline-flex{display:-webkit-inline-box!important;display:-ms-inline-flexbox!important;display:inline-flex!important}}@media (min-width:1200px){.d-xl-none{display:none!important}.d-xl-inline{display:inline!important}.d-xl-inline-block{display:inline-block!important}.d-xl-block{display:block!important}.d-xl-table{display:table!important}.d-xl-table-row{display:table-row!important}.d-xl-table-cell{display:table-cell!important}.d-xl-flex{display:-webkit-box!important;display:-ms-flexbox!important;display:flex!important}.d-xl-inline-flex{display:-webkit-inline-box!important;display:-ms-inline-flexbox!important;display:inline-flex!important}}@media print{.d-print-none{display:none!important}.d-print-inline{display:inline!important}.d-print-inline-block{display:inline-block!important}.d-print-block{display:block!important}.d-print-table{display:table!important}.d-print-table-row{display:table-row!important}.d-print-table-cell{display:table-cell!important}.d-print-flex{display:-webkit-box!important;display:-ms-flexbox!important;display:flex!important}.d-print-inline-flex{display:-webkit-inline-box!important;display:-ms-inline-flexbox!important;display:inline-flex!important}}.flex-row{-webkit-box-orient:horizontal!important;-webkit-box-direction:normal!important;-ms-flex-direction:row!important;flex-direction:row!important}.flex-column{-webkit-box-orient:vertical!important;-webkit-box-direction:normal!important;-ms-flex-direction:column!important;flex-direction:column!important}.flex-row-reverse{-webkit-box-orient:horizontal!important;-webkit-box-direction:reverse!important;-ms-flex-direction:row-reverse!important;flex-direction:row-reverse!important}.flex-column-reverse{-webkit-box-orient:vertical!important;-webkit-box-direction:reverse!important;-ms-flex-direction:column-reverse!important;flex-direction:column-reverse!important}.flex-wrap{-ms-flex-wrap:wrap!important;flex-wrap:wrap!important}.flex-nowrap{-ms-flex-wrap:nowrap!important;flex-wrap:nowrap!important}.flex-wrap-reverse{-ms-flex-wrap:wrap-reverse!important;flex-wrap:wrap-reverse!important}.justify-content-start{-webkit-box-pack:start!important;-ms-flex-pack:start!important;justify-content:flex-start!important}.justify-content-end{-webkit-box-pack:end!important;-ms-flex-pack:end!important;justify-content:flex-end!important}.justify-content-center{-webkit-box-pack:center!important;-ms-flex-pack:center!important;justify-content:center!important}.justify-content-between{-webkit-box-pack:justify!important;-ms-flex-pack:justify!important;justify-content:space-between!important}.justify-content-around{-ms-flex-pack:distribute!important;justify-content:space-around!important}.align-items-start{-webkit-box-align:start!important;-ms-flex-align:start!important;align-items:flex-start!important}.align-items-end{-webkit-box-align:end!important;-ms-flex-align:end!important;align-items:flex-end!important}.align-items-center{-webkit-box-align:center!important;-ms-flex-align:center!important;align-items:center!important}.align-items-baseline{-webkit-box-align:baseline!important;-ms-flex-align:baseline!important;align-items:baseline!important}.align-items-stretch{-webkit-box-align:stretch!important;-ms-flex-align:stretch!important;align-items:stretch!important}.align-content-start{-ms-flex-line-pack:start!important;align-content:flex-start!important}.align-content-end{-ms-flex-line-pack:end!important;align-content:flex-end!important}.align-content-center{-ms-flex-line-pack:center!important;align-content:center!important}.align-content-between{-ms-flex-line-pack:justify!important;align-content:space-between!important}.align-content-around{-ms-flex-line-pack:distribute!important;align-content:space-around!important}.align-content-stretch{-ms-flex-line-pack:stretch!important;align-content:stretch!important}.align-self-auto{-ms-flex-item-align:auto!important;align-self:auto!important}.align-self-start{-ms-flex-item-align:start!important;align-self:flex-start!important}.align-self-end{-ms-flex-item-align:end!important;align-self:flex-end!important}.align-self-center{-ms-flex-item-align:center!important;align-self:center!important}.align-self-baseline{-ms-flex-item-align:baseline!important;align-self:baseline!important}.align-self-stretch{-ms-flex-item-align:stretch!important;align-self:stretch!important}@media (min-width:576px){.flex-sm-row{-webkit-box-orient:horizontal!important;-webkit-box-direction:normal!important;-ms-flex-direction:row!important;flex-direction:row!important}.flex-sm-column{-webkit-box-orient:vertical!important;-webkit-box-direction:normal!important;-ms-flex-direction:column!important;flex-direction:column!important}.flex-sm-row-reverse{-webkit-box-orient:horizontal!important;-webkit-box-direction:reverse!important;-ms-flex-direction:row-reverse!important;flex-direction:row-reverse!important}.flex-sm-column-reverse{-webkit-box-orient:vertical!important;-webkit-box-direction:reverse!important;-ms-flex-direction:column-reverse!important;flex-direction:column-reverse!important}.flex-sm-wrap{-ms-flex-wrap:wrap!important;flex-wrap:wrap!important}.flex-sm-nowrap{-ms-flex-wrap:nowrap!important;flex-wrap:nowrap!important}.flex-sm-wrap-reverse{-ms-flex-wrap:wrap-reverse!important;flex-wrap:wrap-reverse!important}.justify-content-sm-start{-webkit-box-pack:start!important;-ms-flex-pack:start!important;justify-content:flex-start!important}.justify-content-sm-end{-webkit-box-pack:end!important;-ms-flex-pack:end!important;justify-content:flex-end!important}.justify-content-sm-center{-webkit-box-pack:center!important;-ms-flex-pack:center!important;justify-content:center!important}.justify-content-sm-between{-webkit-box-pack:justify!important;-ms-flex-pack:justify!important;justify-content:space-between!important}.justify-content-sm-around{-ms-flex-pack:distribute!important;justify-content:space-around!important}.align-items-sm-start{-webkit-box-align:start!important;-ms-flex-align:start!important;align-items:flex-start!important}.align-items-sm-end{-webkit-box-align:end!important;-ms-flex-align:end!important;align-items:flex-end!important}.align-items-sm-center{-webkit-box-align:center!important;-ms-flex-align:center!important;align-items:center!important}.align-items-sm-baseline{-webkit-box-align:baseline!important;-ms-flex-align:baseline!important;align-items:baseline!important}.align-items-sm-stretch{-webkit-box-align:stretch!important;-ms-flex-align:stretch!important;align-items:stretch!important}.align-content-sm-start{-ms-flex-line-pack:start!important;align-content:flex-start!important}.align-content-sm-end{-ms-flex-line-pack:end!important;align-content:flex-end!important}.align-content-sm-center{-ms-flex-line-pack:center!important;align-content:center!important}.align-content-sm-between{-ms-flex-line-pack:justify!important;align-content:space-between!important}.align-content-sm-around{-ms-flex-line-pack:distribute!important;align-content:space-around!important}.align-content-sm-stretch{-ms-flex-line-pack:stretch!important;align-content:stretch!important}.align-self-sm-auto{-ms-flex-item-align:auto!important;align-self:auto!important}.align-self-sm-start{-ms-flex-item-align:start!important;align-self:flex-start!important}.align-self-sm-end{-ms-flex-item-align:end!important;align-self:flex-end!important}.align-self-sm-center{-ms-flex-item-align:center!important;align-self:center!important}.align-self-sm-baseline{-ms-flex-item-align:baseline!important;align-self:baseline!important}.align-self-sm-stretch{-ms-flex-item-align:stretch!important;align-self:stretch!important}}@media (min-width:768px){.flex-md-row{-webkit-box-orient:horizontal!important;-webkit-box-direction:normal!important;-ms-flex-direction:row!important;flex-direction:row!important}.flex-md-column{-webkit-box-orient:vertical!important;-webkit-box-direction:normal!important;-ms-flex-direction:column!important;flex-direction:column!important}.flex-md-row-reverse{-webkit-box-orient:horizontal!important;-webkit-box-direction:reverse!important;-ms-flex-direction:row-reverse!important;flex-direction:row-reverse!important}.flex-md-column-reverse{-webkit-box-orient:vertical!important;-webkit-box-direction:reverse!important;-ms-flex-direction:column-reverse!important;flex-direction:column-reverse!important}.flex-md-wrap{-ms-flex-wrap:wrap!important;flex-wrap:wrap!important}.flex-md-nowrap{-ms-flex-wrap:nowrap!important;flex-wrap:nowrap!important}.flex-md-wrap-reverse{-ms-flex-wrap:wrap-reverse!important;flex-wrap:wrap-reverse!important}.justify-content-md-start{-webkit-box-pack:start!important;-ms-flex-pack:start!important;justify-content:flex-start!important}.justify-content-md-end{-webkit-box-pack:end!important;-ms-flex-pack:end!important;justify-content:flex-end!important}.justify-content-md-center{-webkit-box-pack:center!important;-ms-flex-pack:center!important;justify-content:center!important}.justify-content-md-between{-webkit-box-pack:justify!important;-ms-flex-pack:justify!important;justify-content:space-between!important}.justify-content-md-around{-ms-flex-pack:distribute!important;justify-content:space-around!important}.align-items-md-start{-webkit-box-align:start!important;-ms-flex-align:start!important;align-items:flex-start!important}.align-items-md-end{-webkit-box-align:end!important;-ms-flex-align:end!important;align-items:flex-end!important}.align-items-md-center{-webkit-box-align:center!important;-ms-flex-align:center!important;align-items:center!important}.align-items-md-baseline{-webkit-box-align:baseline!important;-ms-flex-align:baseline!important;align-items:baseline!important}.align-items-md-stretch{-webkit-box-align:stretch!important;-ms-flex-align:stretch!important;align-items:stretch!important}.align-content-md-start{-ms-flex-line-pack:start!important;align-content:flex-start!important}.align-content-md-end{-ms-flex-line-pack:end!important;align-content:flex-end!important}.align-content-md-center{-ms-flex-line-pack:center!important;align-content:center!important}.align-content-md-between{-ms-flex-line-pack:justify!important;align-content:space-between!important}.align-content-md-around{-ms-flex-line-pack:distribute!important;align-content:space-around!important}.align-content-md-stretch{-ms-flex-line-pack:stretch!important;align-content:stretch!important}.align-self-md-auto{-ms-flex-item-align:auto!important;align-self:auto!important}.align-self-md-start{-ms-flex-item-align:start!important;align-self:flex-start!important}.align-self-md-end{-ms-flex-item-align:end!important;align-self:flex-end!important}.align-self-md-center{-ms-flex-item-align:center!important;align-self:center!important}.align-self-md-baseline{-ms-flex-item-align:baseline!important;align-self:baseline!important}.align-self-md-stretch{-ms-flex-item-align:stretch!important;align-self:stretch!important}}@media (min-width:992px){.flex-lg-row{-webkit-box-orient:horizontal!important;-webkit-box-direction:normal!important;-ms-flex-direction:row!important;flex-direction:row!important}.flex-lg-column{-webkit-box-orient:vertical!important;-webkit-box-direction:normal!important;-ms-flex-direction:column!important;flex-direction:column!important}.flex-lg-row-reverse{-webkit-box-orient:horizontal!important;-webkit-box-direction:reverse!important;-ms-flex-direction:row-reverse!important;flex-direction:row-reverse!important}.flex-lg-column-reverse{-webkit-box-orient:vertical!important;-webkit-box-direction:reverse!important;-ms-flex-direction:column-reverse!important;flex-direction:column-reverse!important}.flex-lg-wrap{-ms-flex-wrap:wrap!important;flex-wrap:wrap!important}.flex-lg-nowrap{-ms-flex-wrap:nowrap!important;flex-wrap:nowrap!important}.flex-lg-wrap-reverse{-ms-flex-wrap:wrap-reverse!important;flex-wrap:wrap-reverse!important}.justify-content-lg-start{-webkit-box-pack:start!important;-ms-flex-pack:start!important;justify-content:flex-start!important}.justify-content-lg-end{-webkit-box-pack:end!important;-ms-flex-pack:end!important;justify-content:flex-end!important}.justify-content-lg-center{-webkit-box-pack:center!important;-ms-flex-pack:center!important;justify-content:center!important}.justify-content-lg-between{-webkit-box-pack:justify!important;-ms-flex-pack:justify!important;justify-content:space-between!important}.justify-content-lg-around{-ms-flex-pack:distribute!important;justify-content:space-around!important}.align-items-lg-start{-webkit-box-align:start!important;-ms-flex-align:start!important;align-items:flex-start!important}.align-items-lg-end{-webkit-box-align:end!important;-ms-flex-align:end!important;align-items:flex-end!important}.align-items-lg-center{-webkit-box-align:center!important;-ms-flex-align:center!important;align-items:center!important}.align-items-lg-baseline{-webkit-box-align:baseline!important;-ms-flex-align:baseline!important;align-items:baseline!important}.align-items-lg-stretch{-webkit-box-align:stretch!important;-ms-flex-align:stretch!important;align-items:stretch!important}.align-content-lg-start{-ms-flex-line-pack:start!important;align-content:flex-start!important}.align-content-lg-end{-ms-flex-line-pack:end!important;align-content:flex-end!important}.align-content-lg-center{-ms-flex-line-pack:center!important;align-content:center!important}.align-content-lg-between{-ms-flex-line-pack:justify!important;align-content:space-between!important}.align-content-lg-around{-ms-flex-line-pack:distribute!important;align-content:space-around!important}.align-content-lg-stretch{-ms-flex-line-pack:stretch!important;align-content:stretch!important}.align-self-lg-auto{-ms-flex-item-align:auto!important;align-self:auto!important}.align-self-lg-start{-ms-flex-item-align:start!important;align-self:flex-start!important}.align-self-lg-end{-ms-flex-item-align:end!important;align-self:flex-end!important}.align-self-lg-center{-ms-flex-item-align:center!important;align-self:center!important}.align-self-lg-baseline{-ms-flex-item-align:baseline!important;align-self:baseline!important}.align-self-lg-stretch{-ms-flex-item-align:stretch!important;align-self:stretch!important}}@media (min-width:1200px){.flex-xl-row{-webkit-box-orient:horizontal!important;-webkit-box-direction:normal!important;-ms-flex-direction:row!important;flex-direction:row!important}.flex-xl-column{-webkit-box-orient:vertical!important;-webkit-box-direction:normal!important;-ms-flex-direction:column!important;flex-direction:column!important}.flex-xl-row-reverse{-webkit-box-orient:horizontal!important;-webkit-box-direction:reverse!important;-ms-flex-direction:row-reverse!important;flex-direction:row-reverse!important}.flex-xl-column-reverse{-webkit-box-orient:vertical!important;-webkit-box-direction:reverse!important;-ms-flex-direction:column-reverse!important;flex-direction:column-reverse!important}.flex-xl-wrap{-ms-flex-wrap:wrap!important;flex-wrap:wrap!important}.flex-xl-nowrap{-ms-flex-wrap:nowrap!important;flex-wrap:nowrap!important}.flex-xl-wrap-reverse{-ms-flex-wrap:wrap-reverse!important;flex-wrap:wrap-reverse!important}.justify-content-xl-start{-webkit-box-pack:start!important;-ms-flex-pack:start!important;justify-content:flex-start!important}.justify-content-xl-end{-webkit-box-pack:end!important;-ms-flex-pack:end!important;justify-content:flex-end!important}.justify-content-xl-center{-webkit-box-pack:center!important;-ms-flex-pack:center!important;justify-content:center!important}.justify-content-xl-between{-webkit-box-pack:justify!important;-ms-flex-pack:justify!important;justify-content:space-between!important}.justify-content-xl-around{-ms-flex-pack:distribute!important;justify-content:space-around!important}.align-items-xl-start{-webkit-box-align:start!important;-ms-flex-align:start!important;align-items:flex-start!important}.align-items-xl-end{-webkit-box-align:end!important;-ms-flex-align:end!important;align-items:flex-end!important}.align-items-xl-center{-webkit-box-align:center!important;-ms-flex-align:center!important;align-items:center!important}.align-items-xl-baseline{-webkit-box-align:baseline!important;-ms-flex-align:baseline!important;align-items:baseline!important}.align-items-xl-stretch{-webkit-box-align:stretch!important;-ms-flex-align:stretch!important;align-items:stretch!important}.align-content-xl-start{-ms-flex-line-pack:start!important;align-content:flex-start!important}.align-content-xl-end{-ms-flex-line-pack:end!important;align-content:flex-end!important}.align-content-xl-center{-ms-flex-line-pack:center!important;align-content:center!important}.align-content-xl-between{-ms-flex-line-pack:justify!important;align-content:space-between!important}.align-content-xl-around{-ms-flex-line-pack:distribute!important;align-content:space-around!important}.align-content-xl-stretch{-ms-flex-line-pack:stretch!important;align-content:stretch!important}.align-self-xl-auto{-ms-flex-item-align:auto!important;align-self:auto!important}.align-self-xl-start{-ms-flex-item-align:start!important;align-self:flex-start!important}.align-self-xl-end{-ms-flex-item-align:end!important;align-self:flex-end!important}.align-self-xl-center{-ms-flex-item-align:center!important;align-self:center!important}.align-self-xl-baseline{-ms-flex-item-align:baseline!important;align-self:baseline!important}.align-self-xl-stretch{-ms-flex-item-align:stretch!important;align-self:stretch!important}}\n" \
	"/*# sourceMappingURL=bootstrap-grid.min.css.map */";

static const char* CSS_REBUTTON = \
	"html, *{font-size: 16px; font-family: -apple-system, BlinkMacSystemFont,\"Segoe UI\",\"Roboto\", \"Droid Sans\",\"Helvetica Neue\", Helvetica, Arial, sans-serif; line-height: 1.5;}" \
	"body{margin: 0; color: #212121; background: #f8f8f8;}" \
	"h1, h2, h3{line-height: 1.2rem; margin: 0.75rem; font-weight: 500;}" \
	"h1{font-size: 1.5rem; margin: 0rem 0.75rem;}" \
	"h2{font-size: 1.25rem;}" \
	"h3{font-size: 1rem;}" \
	"p{margin: 0.75rem;}" \
	"small{font-size: 0.75em;}" \
	"header{height: 3rem; background: #8bc31f; color: #f5f5f5;}" \
	"header h1{line-height: 3rem;}" \
	".row{margin: 0.5rem;}" \
	"form{background: #eeeeee; border: 1px solid #c9c9c9; margin: 0.5rem; padding: 1rem;}" \
	"input{width:100%;}" \
	"button{background: rgba(143, 195, 31, 1); color: #fafafa; border: 0; border-radius: 2px; padding: 0.5rem 0.75rem; margin: 0.5rem; text-decoration: none; transition: background 0.3s; cursor: pointer;}" \
	"button:hover{background: #004966; opacity: 1;}";

static const char* HTML_INDEX = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<link rel=\"stylesheet\" href=\"/bootstrap-grid.min.css\" crossorigin=\"anonymous\">" \
			"<link rel=\"stylesheet\" href=\"/rebutton.css\" crossorigin=\"anonymous\">" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Home</h1>" \
			"</header>" \
			"<div class=\"container\">" \
				"<div class=\"row\"><p style=\"width: 100%%; text-align: center;\"><a href=\"/wifi\" target=\"_self\">Wi-Fi</a></p></div>" \
				"<div class=\"row\"><p style=\"width: 100%%; text-align: center;\"><a href=\"/iotcentral\" target=\"_self\">Azure IoT Central</a></p></div>" \
				"<div class=\"row\"><p style=\"width: 100%%; text-align: center;\"><a href=\"/iothub\" target=\"_self\">Azure IoT Hub</a></p></div>" \
				"<div class=\"row\"><p style=\"width: 100%%; text-align: center;\"><a href=\"/message\" target=\"_self\">Device to Cloud (D2C) Message</a></p></div>" \
				"<div class=\"row\"><p style=\"width: 100%%; text-align: center;\"><a href=\"/apmode\" target=\"_self\">Access Point Mode</a></p></div>" \
				"<div class=\"row\"><p style=\"width: 100%%; text-align: center;\"><a href=\"/firmware\" target=\"_self\">Firmware Update</a></p></div>" \
				"<div class=\"row\">" \
					"<button class=\"col-md-2 offset-md-9\" type=\"button\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
				"</div>" \
				"<div style=\"background: #e0e0e0;\">" \
					"<div class=\"row\">" \
						"<small class=\"col-md-2\">MAC Address</small>" \
						"<small class=\"col-md-10\">%s</small>" \
					"</div>" \
				"</div>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_WIFI_A = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<link rel=\"stylesheet\" href=\"/bootstrap-grid.min.css\" crossorigin=\"anonymous\">" \
			"<link rel=\"stylesheet\" href=\"/rebutton.css\" crossorigin=\"anonymous\">" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Wi-Fi</h1>" \
			"</header>" \
			"<div class=\"container\">" \
				"<form action=\"wifi2\" method=\"post\" enctype=\"multipart/form-data\">" \
					"<div class=\"row\">" \
						"<label class=\"col-md-2\" for=\"WiFiSSID\">Wi-Fi SSID</label>" \
						"<div class=\"col-md-10\">" \
							"<select class=\"col-md-10\" name=\"WiFiSSID\" id=\"WiFiSSID\">";
static const char* HTML_WIFI_B = \
							"</select>" \
						"</div>" \
						"<small class=\"col-md-10 offset-md-2\">" \
							"Select WiFi SSID to connect<br>" \
   							"Please refresh browser to re-scan SSID if your SSID is not in the list above" \
						"</small>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-md-2\" for=\"WiFiPassword\">Wi-Fi Passphrase</label>" \
						"<div class=\"col-md-10\">" \
							"<input name=\"WiFiPassword\" id=\"WiFiPassword\" value=\"\" type=\"password\">" \
						"</div>" \
						"<small class=\"col-md-10 offset-md-2\">Specify passphrase for SSID</small>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-md-2\" for=\"TimeServer\">Time Server</label>" \
						"<div class=\"col-md-10\">" \
							"<input name=\"TimeServer\" id=\"TimeServer\" value=\"%s\" type=\"text\">" \
						"</div>" \
						"<small class=\"col-md-10 offset-md-2\">Specify Internet Time Server (Optional)</small>" \
					"</div>" \
					"<div class=\"row\">" \
						"<button class=\"col-md-2 offset-md-1\" type=\"submit\">Save</button>" \
						"<button class=\"col-md-2 offset-md-3\" type=\"button\" onclick=\"location.href='/'\">Home</button>" \
						"<button class=\"col-md-2\" type=\"button\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
					"</div>" \
				"</form>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_WIFI2_SUCCESS = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<link rel=\"stylesheet\" href=\"/bootstrap-grid.min.css\" crossorigin=\"anonymous\">" \
			"<link rel=\"stylesheet\" href=\"/rebutton.css\" crossorigin=\"anonymous\">" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Wi-Fi</h1>" \
			"</header>" \
			"<div class=\"container\">" \
				"<div class=\"row\">" \
					"<h2 style=\"width: 100%%; text-align: center;\">Wi-Fi saved.</h2>" \
				"</div>" \
				"<div class=\"row\">" \
					"<button class=\"col-md-2 offset-md-7\" type=\"button\" onclick=\"location.href='/'\">Home</button>" \
					"<button class=\"col-md-2\" type=\"button\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
				"</div>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_IOTCENTRAL = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<link rel=\"stylesheet\" href=\"/bootstrap-grid.min.css\" crossorigin=\"anonymous\">" \
			"<link rel=\"stylesheet\" href=\"/rebutton.css\" crossorigin=\"anonymous\">" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Azure IoT Central</h1>" \
			"</header>" \
			"<div class=\"container\">" \
				"<form action=\"iotcentral2\" method=\"post\" enctype=\"multipart/form-data\">" \
					"<div class=\"row\">" \
						"<h2>Connect ReButton to Azure IoT Central</h2>" \
					"</div>" \
					"<div class=\"row\">" \
						"<p>" \
							"More about Azure IoT Central : <a href=\"https://aka.ms/AzureIoTCentral\">https://aka.ms/AzureIoTCentral</a><br>" \
							"Please erase IoT Hub Connection String to connect to Azure IoT Central" \
						"</p>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-md-2\" for=\"ScopeId\">Scope ID</label>" \
						"<div class=\"col-md-10\">" \
							"<input name=\"ScopeId\" id=\"ScopeId\" value=\"%s\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-md-2\" for=\"DeviceId\">Device ID</label>" \
						"<div class=\"col-md-10\">" \
							"<input name=\"DeviceId\" id=\"DeviceId\" value=\"%s\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-md-2\" for=\"SasKey\">SAS Key</label>" \
						"<div class=\"col-md-10\">" \
							"<input name=\"SasKey\" id=\"SasKey\" value=\"%s\" type=\"text\">" \
						"</div>" \
						"<small class=\"col-md-10 offset-md-2\">Please enter either Primary or Secondary key</small>" \
					"</div>" \
					"<div class=\"row\">" \
						"<button class=\"col-md-2 offset-md-1\" type=\"submit\">Save</button>" \
						"<button class=\"col-md-2 offset-md-3\" type=\"button\" onclick=\"location.href='/'\">Home</button>" \
						"<button class=\"col-md-2\" type=\"button\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
					"</div>" \
				"</form>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_IOTCENTRAL2_SUCCESS = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<link rel=\"stylesheet\" href=\"/bootstrap-grid.min.css\" crossorigin=\"anonymous\">" \
			"<link rel=\"stylesheet\" href=\"/rebutton.css\" crossorigin=\"anonymous\">" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Azure IoT Central</h1>" \
			"</header>" \
			"<div class=\"container\">" \
				"<div class=\"row\">" \
					"<h2 style=\"width: 100%%; text-align: center;\">Azure IoT Central saved.</h2>" \
				"</div>" \
				"<div class=\"row\">" \
					"<button class=\"col-md-2 offset-md-7\" type=\"button\" onclick=\"location.href='/'\">Home</button>" \
					"<button class=\"col-md-2\" type=\"button\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
				"</div>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_IOTHUB = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<link rel=\"stylesheet\" href=\"/bootstrap-grid.min.css\" crossorigin=\"anonymous\">" \
			"<link rel=\"stylesheet\" href=\"/rebutton.css\" crossorigin=\"anonymous\">" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Azure IoT Hub</h1>" \
			"</header>" \
			"<div class=\"container\">" \
				"<form action=\"iothub2\" method=\"post\" enctype=\"multipart/form-data\"> " \
					"<div class=\"row\">" \
						"<h2>Connect ReButton to Azure IoT Hub</h2>" \
					"</div>" \
					"<div class=\"row\">" \
						"<p>" \
							"More about Azure IoT Hub : <a href=\"https://aka.ms/iothub\">https://aka.ms/iothub</a>" \
						"</p>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-12\" for=\"IoTHubConnectionString\">Azure IoT Hub connection string</label>" \
						"<div class=\"col-12\">" \
							"<input name=\"IoTHubConnectionString\" id=\"IoTHubConnectionString\" value=\"%s\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<button class=\"col-md-2 offset-md-1\" type=\"submit\">Save</button>" \
						"<button class=\"col-md-2 offset-md-3\" type=\"button\" onclick=\"location.href='/'\">Home</button>" \
						"<button class=\"col-md-2\" type=\"button\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
					"</div>" \
				"</form>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_IOTHUB2_SUCCESS = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<link rel=\"stylesheet\" href=\"/bootstrap-grid.min.css\" crossorigin=\"anonymous\">" \
			"<link rel=\"stylesheet\" href=\"/rebutton.css\" crossorigin=\"anonymous\">" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Azure IoT Hub</h1>" \
			"</header>" \
			"<div class=\"container\">" \
				"<div class=\"row\">" \
					"<h2 style=\"width: 100%%; text-align: center;\">Azure IoT Hub saved.</h2>" \
				"</div>" \
				"<div class=\"row\">" \
					"<button class=\"col-md-2 offset-md-7\" type=\"button\" onclick=\"location.href='/'\">Home</button>" \
					"<button class=\"col-md-2\" type=\"button\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
				"</div>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_MESSAGE = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<link rel=\"stylesheet\" href=\"/bootstrap-grid.min.css\" crossorigin=\"anonymous\">" \
			"<link rel=\"stylesheet\" href=\"/rebutton.css\" crossorigin=\"anonymous\">" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Message</h1>" \
			"</header>" \
			"<div class=\"container\">" \
				"<form action=\"message2\" method=\"post\" enctype=\"multipart/form-data\">" \
					"<div class=\"row\">" \
						"<h2>Message Settings</h2>" \
					"</div>" \
					"<div class=\"row\">" \
						"<p>" \
							"Customize messages for button press events" \
						"</p>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-12\" for=\"Message1\">Single click message</label>" \
						"<div class=\"col-12\">" \
							"<input name=\"Message1\" id=\"Message1\" value=\"%s\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-12\" for=\"Message2\">Double click message</label>" \
						"<div class=\"col-12\">" \
							"<input name=\"Message2\" id=\"Message2\" value=\"%s\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-12\" for=\"Message3\">Triple click message</label>" \
						"<div class=\"col-12\">" \
							"<input name=\"Message3\" id=\"Message3\" value=\"%s\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-12\" for=\"Message10\">Long press message (3 seconds)</label>" \
						"<div class=\"col-12\">" \
							"<input name=\"Message10\" id=\"Message10\" value=\"%s\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-12\" for=\"Message11\">Super long press message (6 Seconds)</label>" \
						"<div class=\"col-12\">" \
							"<input name=\"Message11\" id=\"Message11\" value=\"%s\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-12\" for=\"CustomMessageJson\">Custom message (JSON format)</label>" \
						"<div class=\"col-12\">" \
							"<input name=\"CustomMessageJson\" id=\"CustomMessageJson\" value=\"%s\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-12\" for=\"ProductId\">Product Id</label>" \
						"<div class=\"col-12\">" \
							"<input name=\"ProductId\" id=\"ProductId\" value=\"%s\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-12\" for=\"CustomMessagePropName\">Custom message property name (Device twins)</label>" \
						"<div class=\"col-12\">" \
							"<input name=\"CustomMessagePropName\" id=\"CustomMessagePropName\" value=\"%s\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<button class=\"col-md-2 offset-md-1\" type=\"submit\">Save</button>" \
						"<button class=\"col-md-2 offset-md-3\" type=\"button\" onclick=\"location.href='/'\">Home</button>" \
						"<button class=\"col-md-2\" type=\"button\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
					"</div>" \
				"</form>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_MESSAGE2_SUCCESS = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<link rel=\"stylesheet\" href=\"/bootstrap-grid.min.css\" crossorigin=\"anonymous\">" \
			"<link rel=\"stylesheet\" href=\"/rebutton.css\" crossorigin=\"anonymous\">" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Message</h1> " \
			"</header>" \
			"<div class=\"container\">" \
				"<div class=\"row\">" \
					"<h2 style=\"width: 100%%; text-align: center;\">Message saved.</h2>" \
				"</div>" \
				"<div class=\"row\">" \
					"<button class=\"col-md-2 offset-md-7\" type=\"button\" onclick=\"location.href='/'\">Home</button>" \
					"<button class=\"col-md-2\" type=\"button\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
				"</div>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_APMODE = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<link rel=\"stylesheet\" href=\"/bootstrap-grid.min.css\" crossorigin=\"anonymous\">" \
			"<link rel=\"stylesheet\" href=\"/rebutton.css\" crossorigin=\"anonymous\">" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Access Point Mode</h1>" \
			"</header>" \
			"<div class=\"container\">" \
				"<form action=\"apmode2\" method=\"post\" enctype=\"multipart/form-data\">" \
					"<div class=\"row\">" \
						"<h2>Access Point Mode Settings</h2>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-md-2\" for=\"APmodeSSID\">AP SSID</label>" \
						"<div class=\"col-md-10\">" \
							"<input name=\"APmodeSSID\" id=\"APmodeSSID\" value=\"%s\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<button class=\"col-md-2 offset-md-1\" type=\"submit\">Save</button>" \
						"<button class=\"col-md-2 offset-md-3\" type=\"button\" onclick=\"location.href='/'\">Home</button>" \
						"<button class=\"col-md-2\" type=\"button\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
					"</div>" \
				"</form>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_APMODE2_SUCCESS = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<link rel=\"stylesheet\" href=\"/bootstrap-grid.min.css\" crossorigin=\"anonymous\">" \
			"<link rel=\"stylesheet\" href=\"/rebutton.css\" crossorigin=\"anonymous\">" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Access Point Mode</h1> " \
			"</header>" \
			"<div class=\"container\">" \
				"<div class=\"row\">" \
					"<h2 style=\"width: 100%%; text-align: center;\">Access Point Mode saved.</h2>" \
				"</div>" \
				"<div class=\"row\">" \
					"<button class=\"col-md-2 offset-md-7\" type=\"button\" onclick=\"location.href='/'\">Home</button>" \
					"<button class=\"col-md-2\" type=\"button\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
				"</div>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_FIRMWARE_UPDATE = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<link rel=\"stylesheet\" href=\"/bootstrap-grid.min.css\" crossorigin=\"anonymous\">" \
			"<link rel=\"stylesheet\" href=\"/rebutton.css\" crossorigin=\"anonymous\">" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Firmware Update</h1>" \
			"</header>" \
			"<div class=\"container\">" \
				"<form action=\"firmware2\" method=\"post\" enctype=\"multipart/form-data\"> " \
					"<div class=\"row\">" \
						"<h2>Current firmware version : %s</h2>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-md-2\" for=\"PackageURI\">Package URI</label>" \
						"<div class=\"col-md-10\">" \
							"<input name=\"PackageURI\" id=\"PackageURI\" value=\"\" type=\"url\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-md-2\" for=\"PackageCRC\">Package CRC</label>" \
						"<div class=\"col-md-10\">" \
							"<input name=\"PackageCRC\" id=\"PackageCRC\" value=\"\" type=\"text\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<label class=\"col-md-2\" for=\"PackageSize\">Package size</label>" \
						"<div class=\"col-md-10\">" \
							"<input name=\"PackageSize\" id=\"PackageSize\" value=\"\" type=\"number\">" \
						"</div>" \
					"</div>" \
					"<div class=\"row\">" \
						"<button class=\"col-md-2 offset-md-1\" type=\"submit\">Update</button>" \
						"<button class=\"col-md-2 offset-md-3\" type=\"button\" onclick=\"location.href='/'\">Home</button>" \
						"<button class=\"col-md-2\" type=\"button\" onclick=\"location.href='/shutdown'\">Shutdown</button>" \
					"</div>" \
				"</form>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_FIRMWARE_UPDATE2 = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<style>%s</style>" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Firmware Update</h1>" \
			"</header>" \
			"<div class=\"container\">" \
				"<div class=\"row\">" \
					"<h2 style=\"width: 100%%; text-align: center; color:#2e7d32;\">Firmware updating...</h2>" \
				"</div>" \
				"<div class=\"row\">" \
					"<h5 style=\"width: 100%%; text-align: center;\">Wait a few seconds for the ReButton to shutdown...</h5>" \
				"</div>" \
			"</div>" \
		"</body>" \
	"</html>";

static const char* HTML_SHUTDOWN = \
	"<!DOCTYPE html>" \
	"<html lang=\"en\">" \
		"<head>" \
			"<meta charset=\"UTF-8\">" \
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">" \
			"<meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">" \
			"<title>ReButton</title>" \
			"<style>%s</style>" \
		"</head>" \
		"<body>" \
			"<header>" \
				"<h1>ReButton - Shutdown</h1>" \
			"</header>" \
			"<div class=\"container\">" \
				"<div class=\"row\">" \
					"<h2 style=\"width: 100%%; text-align: center; color:#2e7d32;\">Shutdown</h2>" \
				"</div>" \
			"</div>" \
		"</body>" \
	"</html>";

static bool is_http_init;
static bool is_handlers_registered;

static String stringformat(int maxLength, const char* format, ...)
{
	va_list args;
	char* buf = new char[maxLength + 1];

	va_start(args, format);
	vsnprintf(buf, maxLength + 1, format, args);
	va_end(args);

	String str(buf);

	delete[] buf;

	return str;
}

static int write_eeprom(char* string, int idxZone)
{
	Serial.printf("write_eeprom(\"%s\",%d)\n", string, idxZone);
	return 0;
}

static String FormValueEncode(const char* text)
{
	String html;

	while (*text != '\0')
	{
		switch (*text)
		{
		case '"':
			html += "&quot;";
			break;
		case '&':
			html += "&amp;";
			break;
		case '<':
			html += "&lt;";
			break;
		case '>':
			html += "&gt;";
			break;
		default:
			html += *text;
		}
		text++;
	}

	return html;
}

static OSStatus HttpdSend(httpd_request_t* req, const char* content)
{
	OSStatus err;

	err = httpd_send_all_header(req, HTTP_RES_200, strlen(content), HTTP_CONTENT_HTML_STR);
	require_noerr_action(err, exit, app_httpd_log("ERROR: Unable to send http result headers."));

	err = httpd_send_body(req->sock, (const unsigned char*)content, strlen(content));
	require_noerr_action(err, exit, app_httpd_log("ERROR: Unable to send http result body."));

exit:
	return err;
}

static OSStatus HttpdSendCss(httpd_request_t* req, const char* content)
{
	OSStatus err;

	err = httpd_send_all_header(req, HTTP_RES_200, strlen(content), HTTP_CONTENT_CSS_STR);
	require_noerr_action(err, exit, app_httpd_log("ERROR: Unable to send http result headers."));

	err = httpd_send_body(req->sock, (const unsigned char*)content, strlen(content));
	require_noerr_action(err, exit, app_httpd_log("ERROR: Unable to send http result body."));

exit:
	return err;
}

////////////////////////////////////////////////////////////////////////////////////
// HTML handlers

static int CssBootstrapGrid(httpd_request_t* req)
{
	OSStatus err;

	if ((err = HttpdSendCss(req, CSS_BOOTSTRAP_GRID)) != kNoErr) return err;

	return kNoErr;
}

static int CssRebutton(httpd_request_t* req)
{
	OSStatus err;

	if ((err = HttpdSendCss(req, CSS_REBUTTON)) != kNoErr) return err;

	return kNoErr;
}

static int HtmlIndexGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	unsigned char macAddress[6];
	WiFi.macAddress(macAddress);
	char strMacAddress[6 * 3 + 1];
	snprintf(strMacAddress, sizeof(strMacAddress), "%02x:%02x:%02x:%02x:%02x:%02x", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);

	String html = stringformat(strlen(HTML_INDEX) + strlen(strMacAddress), HTML_INDEX, strMacAddress);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlWiFiGetHandler(httpd_request_t *req)
{
	AutoShutdownUpdateStartTime();

	int err;

	// scan network
	WiFiAccessPoint wifiScanResult[100];
	int wifiCount = ((EMW10xxInterface*)network)->scan(wifiScanResult, 100);

	int validWifiIndex[100];
	int validWifiCount = 0;
	for (int i = 0; i < wifiCount; i++)
	{
		// too weak
		if (wifiScanResult[i].get_rssi() < -100) continue;

		char* ssid = (char*)wifiScanResult[i].get_ssid();
		int ssidLen = strlen(ssid);

		if (strcmp(ssid, Config.APmodeSSID) == 0) continue;
		if (ssidLen < 1 || CONFIG_WIFI_SSID_MAX_LEN < ssidLen) continue;

		bool shouldSkip = false;
		for (int j = 0; j < i; j++)
		{
			if (strcmp(ssid, wifiScanResult[j].get_ssid()) == 0)
			{
				// duplicate ap name
				shouldSkip = true;
				break;
			}
		}
		if (shouldSkip) continue;

		validWifiIndex[validWifiCount++] = i;
		if (validWifiCount >= sizeof(validWifiIndex) / sizeof(validWifiIndex[0])) break;
	}

	String html = HTML_WIFI_A;
	for (int i = 0; i < validWifiCount; i++)
	{
		char* ssid = (char*)wifiScanResult[validWifiIndex[i]].get_ssid();
		int ssidLen = strlen(ssid);
		html += stringformat(15 + ssidLen + 2 + ssidLen + 9, "<option value=\"%s\">%s</option>", ssid, ssid);
	}
	html += stringformat(strlen(HTML_WIFI_B) + strlen(Config.TimeServer), HTML_WIFI_B, Config.TimeServer);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlWiFi2PostHandler(httpd_request_t *req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	std::vector<char> buf(1000);

	if ((err = httpd_get_data(req, &buf[0], buf.size())) != kNoErr) return err;
	if (strstr(req->content_type, "multipart/form-data") == NULL) return kGeneralErr;
	char* boundary = strstr(req->content_type, "boundary=");
	boundary += 9;

	char wifiSSID[CONFIG_WIFI_SSID_MAX_LEN + 1];
	char wifiPassword[CONFIG_WIFI_PASSWORD_MAX_LEN + 1];
	char timeServer[CONFIG_TIME_SERVER_MAX_LEN + 1];

	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "WiFiSSID", wifiSSID, CONFIG_WIFI_SSID_MAX_LEN)) != kNoErr) return err;
	if (strlen(wifiSSID) <= 0) return kGeneralErr;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "WiFiPassword", wifiPassword, CONFIG_WIFI_PASSWORD_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "TimeServer", timeServer, sizeof(timeServer))) != kNoErr) return err;

	strncpy(Config.WiFiSSID, wifiSSID, sizeof(Config.WiFiSSID));
	strncpy(Config.WiFiPassword, wifiPassword, sizeof(Config.WiFiPassword));
	strncpy(Config.TimeServer, timeServer, sizeof(Config.TimeServer));
	ConfigWrite();

	String html = stringformat(strlen(HTML_WIFI2_SUCCESS), HTML_WIFI2_SUCCESS);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlIoTCentralGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	String html = stringformat(strlen(HTML_IOTCENTRAL) + strlen(Config.ScopeId) + strlen(Config.DeviceId) + strlen(Config.SasKey), HTML_IOTCENTRAL, Config.ScopeId, Config.DeviceId, Config.SasKey);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlIoTCentral2PostHandler(httpd_request_t *req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	std::vector<char> buf(1000);

	if ((err = httpd_get_data(req, &buf[0], buf.size())) != kNoErr) return err;
	if (strstr(req->content_type, "multipart/form-data") == NULL) return kGeneralErr;
	char* boundary = strstr(req->content_type, "boundary=");
	boundary += 9;

	char scopeId[CONFIG_SCOPE_ID_MAX_LEN + 1];
	char deviceId[CONFIG_DEVICE_ID_MAX_LEN + 1];
	char sasKey[CONFIG_SAS_KEY_MAX_LEN + 1];

	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "ScopeId", scopeId, CONFIG_SCOPE_ID_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "DeviceId", deviceId, CONFIG_DEVICE_ID_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "SasKey", sasKey, CONFIG_SAS_KEY_MAX_LEN)) != kNoErr) return err;

	strncpy(Config.ScopeId, scopeId, sizeof(Config.ScopeId));
	strncpy(Config.DeviceId, deviceId, sizeof(Config.DeviceId));
	strncpy(Config.SasKey, sasKey, sizeof(Config.SasKey));
	ConfigWrite();

	String html = stringformat(strlen(HTML_IOTCENTRAL2_SUCCESS), HTML_IOTCENTRAL2_SUCCESS);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlIoTHubGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	String html = stringformat(strlen(HTML_IOTHUB) + strlen(Config.IoTHubConnectionString), HTML_IOTHUB, Config.IoTHubConnectionString);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlIoTHub2PostHandler(httpd_request_t *req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	std::vector<char> buf(1000);

	if ((err = httpd_get_data(req, &buf[0], buf.size())) != kNoErr) return err;
	if (strstr(req->content_type, "multipart/form-data") == NULL) return kGeneralErr;
	char* boundary = strstr(req->content_type, "boundary=");
	boundary += 9;

	char connectionString[CONFIG_IOTHUB_CONNECTION_STRING_MAX_LEN + 1];

	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "IoTHubConnectionString", connectionString, CONFIG_IOTHUB_CONNECTION_STRING_MAX_LEN)) != kNoErr) return err;

	strncpy(Config.IoTHubConnectionString, connectionString, sizeof(Config.IoTHubConnectionString));
	ConfigWrite();

	String html = stringformat(strlen(HTML_IOTHUB2_SUCCESS), HTML_IOTHUB2_SUCCESS);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlMessageGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	String html = stringformat(
		strlen(HTML_MESSAGE) +
		strlen(Config.Message1) + 
		strlen(Config.Message2) + 
		strlen(Config.Message3) + 
		strlen(Config.Message10) + 
		strlen(Config.Message11) + 
		strlen(Config.CustomMessageJson) + 
		strlen(Config.ProductId) + 
		strlen(Config.CustomMessagePropertyName),
		HTML_MESSAGE,
		FormValueEncode(Config.Message1).c_str(),
		FormValueEncode(Config.Message2).c_str(),
		FormValueEncode(Config.Message3).c_str(),
		FormValueEncode(Config.Message10).c_str(),
		FormValueEncode(Config.Message11).c_str(),
		FormValueEncode(Config.CustomMessageJson).c_str(), 
		FormValueEncode(Config.ProductId).c_str(),
			FormValueEncode(Config.CustomMessagePropertyName).c_str()
	);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlMessage2PostHandler(httpd_request_t *req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	std::vector<char> buf(1000);

	if ((err = httpd_get_data(req, &buf[0], buf.size())) != kNoErr) return err;
	if (strstr(req->content_type, "multipart/form-data") == NULL) return kGeneralErr;
	char* boundary = strstr(req->content_type, "boundary=");
	boundary += 9;

	char message1[CONFIG_MESSAGE_MAX_LEN + 1];
	char message2[CONFIG_MESSAGE_MAX_LEN + 1];
	char message3[CONFIG_MESSAGE_MAX_LEN + 1];
	char message10[CONFIG_MESSAGE_MAX_LEN + 1];
	char message11[CONFIG_MESSAGE_MAX_LEN + 1];
	char customMessageJson[CONFIG_CUSTOM_MESSAGE_JSON_MAX_LEN + 1];
	char productId[CONFIG_PRODUCT_ID_MAX_LEN + 1];
	char customMessagePropertyName[CONFIG_PROPERTY_NAME_MAX_LEN + 1];

	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "Message1", message1, CONFIG_MESSAGE_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "Message2", message2, CONFIG_MESSAGE_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "Message3", message3, CONFIG_MESSAGE_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "Message10", message10, CONFIG_MESSAGE_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "Message11", message11, CONFIG_MESSAGE_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "CustomMessageJson", customMessageJson, CONFIG_CUSTOM_MESSAGE_JSON_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "ProductId", productId, CONFIG_PRODUCT_ID_MAX_LEN)) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "CustomMessagePropName", customMessagePropertyName, CONFIG_PROPERTY_NAME_MAX_LEN)) != kNoErr) return err;

	strncpy(Config.Message1, message1, sizeof(Config.Message1));
	strncpy(Config.Message2, message2, sizeof(Config.Message2));
	strncpy(Config.Message3, message3, sizeof(Config.Message3));
	strncpy(Config.Message10, message10, sizeof(Config.Message10));
	strncpy(Config.Message11, message11, sizeof(Config.Message11));
	strncpy(Config.CustomMessageJson, customMessageJson, sizeof(Config.CustomMessageJson));
	strncpy(Config.ProductId, productId, sizeof(Config.ProductId));
	strncpy(Config.CustomMessagePropertyName, customMessagePropertyName, sizeof(Config.CustomMessagePropertyName));
	ConfigWrite();

	String html = stringformat(strlen(HTML_MESSAGE2_SUCCESS), HTML_MESSAGE2_SUCCESS);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlAPmodeGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	String html = stringformat(strlen(HTML_APMODE) + strlen(Config.APmodeSSID), HTML_APMODE, Config.APmodeSSID);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlAPmode2PostHandler(httpd_request_t *req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	std::vector<char> buf(1000);

	if ((err = httpd_get_data(req, &buf[0], buf.size())) != kNoErr) return err;
	if (strstr(req->content_type, "multipart/form-data") == NULL) return kGeneralErr;
	char* boundary = strstr(req->content_type, "boundary=");
	boundary += 9;

	char apmodeSSID[CONFIG_APMODE_SSID_MAX_LEN + 1];

	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "APmodeSSID", apmodeSSID, CONFIG_APMODE_SSID_MAX_LEN)) != kNoErr) return err;

	strncpy(Config.APmodeSSID, apmodeSSID, sizeof(Config.APmodeSSID));
	ConfigWrite();

	String html = stringformat(strlen(HTML_APMODE2_SUCCESS), HTML_APMODE2_SUCCESS);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlFirmwareUpdateGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	String html = stringformat(strlen(HTML_FIRMWARE_UPDATE) + strlen(CONFIG_FIRMWARE_VERSION), HTML_FIRMWARE_UPDATE, CONFIG_FIRMWARE_VERSION);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	return kNoErr;
}

static int HtmlFirmwareUpdate2PostHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	if (!ReButton::IsButtonPressed())
	{
		String html = stringformat(strlen(HTML_FIRMWARE_UPDATE) + strlen(CONFIG_FIRMWARE_VERSION), HTML_FIRMWARE_UPDATE, CONFIG_FIRMWARE_VERSION);
		if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

		return kNoErr;
	}

	std::vector<char> buf(1000);

	if ((err = httpd_get_data(req, &buf[0], buf.size())) != kNoErr) return err;
	if (strstr(req->content_type, "multipart/form-data") == NULL) return kGeneralErr;
	char* boundary = strstr(req->content_type, "boundary=");
	boundary += 9;

	char packageURI[100 + 1];
	char packageCRC[6 + 1];
	char packageSize[6 + 1];

	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "PackageURI", packageURI, sizeof (packageURI))) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "PackageCRC", packageCRC, sizeof(packageCRC))) != kNoErr) return err;
	if ((err = httpd_get_tag_from_multipart_form(&buf[0], boundary, "PackageSize", packageSize, sizeof(packageSize))) != kNoErr) return err;

	Serial.printf("packageURI = %s\n", packageURI);
	Serial.printf("packageCRC = %s\n", packageCRC);
	Serial.printf("packageSize = %s\n", packageSize);

	char* endp;
	int inPackageCRC = strtol(packageCRC, &endp, 0);
	if (*endp != '\0') return kGeneralErr;
	int inPackageSize = strtol(packageSize, &endp, 0);
	if (*endp != '\0') return kGeneralErr;

	String html = stringformat(strlen(HTML_FIRMWARE_UPDATE2) + strlen(CSS_REBUTTON), HTML_FIRMWARE_UPDATE2, CSS_REBUTTON);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	DisplayColor(DISPLAY_OFF);
	do
	{
		Serial.println("Connect WiFi...");
		if (WiFi.begin(Config.WiFiSSID, Config.WiFiPassword) != WL_CONNECTED) break;
		Serial.println("Connected WiFi.");

		Serial.println("Download firmware...");
		uint16_t dlPackageCRC;
		int dlPackageSize = OTADownloadFirmware(packageURI, &dlPackageCRC);
		Serial.printf("Downloaded firmware.\n");
		Serial.printf("inPackageCRC = %#04x\n", inPackageCRC);
		Serial.printf("inPackageSize = %d\n", inPackageSize);
		Serial.printf("dlPackageCRC = %#04x\n", dlPackageCRC);
		Serial.printf("dlPackageSize = %d\n", dlPackageSize);

		if (dlPackageCRC != inPackageCRC || dlPackageSize != inPackageSize) break;

		Serial.println("Switch firmware...");
		if (OTAApplyNewFirmware(inPackageSize, inPackageCRC) != 0) break;
		Serial.println("Switched firmware.");

		delay(1000);
		mico_system_reboot();
	}
	while (false);

	for (;;)
	{
		for (int i = 0; i < 3; i++)
		{
			DisplayColor(DISPLAY_ERROR);
			delay(200);
			DisplayColor(DISPLAY_OFF);
			delay(200);
		}
		ReButton::PowerSupplyEnable(false);
		delay(POWER_OFF_TIME);
	}
}

static int HtmlShutdownGetHandler(httpd_request_t* req)
{
	AutoShutdownUpdateStartTime();

	OSStatus err;

	String html = stringformat(strlen(HTML_SHUTDOWN) + strlen(CSS_REBUTTON), HTML_SHUTDOWN, CSS_REBUTTON);
	if ((err = HttpdSend(req, html.c_str())) != kNoErr) return err;

	AutoShutdownSetTimeout(5000);

	return kNoErr;
}

////////////////////////////////////////////////////////////////////////////////////
// REST handlers

static int RestConfigGetHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  int err = kNoErr;
  return err;
}

static int RestConfigPostHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  OSStatus err = kNoErr;
  return err;  
}

/*
* REST API for IoT Hub
*/
static char *GetHostNameFromConnectionString(char *connectionString)
{
    if (connectionString == NULL)
    {
        return NULL;
    }
    int start = 0;
    int cur = 0;
    bool find = false;
    while (connectionString[cur] > 0)
    {
        if (connectionString[cur] == '=')
        {
            // Check the key
            if (memcmp(&connectionString[start], "HostName", 8) == 0)
            {
                // This is the host name
                find = true;
            }
            start = ++cur;
            // Value
            while (connectionString[cur] > 0)
            {
                if (connectionString[cur] == ';')
                {
                    break;
                }
                cur++;
            }
            if (find && cur - start > 0)
            {
                char *hostname = (char *)malloc(cur - start + 1);
                memcpy(hostname, &connectionString[start], cur - start);
                hostname[cur - start] = 0;
                return hostname;
            }
            start = cur + 1;
        }
        cur++;
    }
    return NULL;
}

static char *GetDeviceNameFromConnectionString(char *connectionString)
{
    if (connectionString == NULL)
    {
        return NULL;
    }
    int start = 0;
    int cur = 0;
    bool find = false;
    while (connectionString[cur] > 0)
    {
        if (connectionString[cur] == '=')
        {
            // Check the key
            if (memcmp(&connectionString[start], "DeviceId", 8) == 0)
            {
                // This is the device id
                find = true;
            }
            start = ++cur;
            // Value
            while (connectionString[cur] > 0)
            {
                if (connectionString[cur] == ';')
                {
                    break;
                }
                cur++;
            }
            if (find && cur - start > 0)
            {
                char *deviceName = (char *)malloc(cur - start + 1);
                memcpy(deviceName, &connectionString[start], cur - start);
                deviceName[cur - start] = 0;
                return deviceName;
            }
            start = cur + 1;
        }
        cur++;
    }
    return NULL;
}

static int RestIoTHubGetHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  EEPROMInterface eeprom;
  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *serialized_string = NULL;
  char *connString[AZ_IOT_HUB_MAX_LEN + 1] = { '\0' };
  int err = kNoErr;

  int ret = eeprom.read((uint8_t*)connString, AZ_IOT_HUB_MAX_LEN, 0x00, AZ_IOT_HUB_ZONE_IDX);

  if (ret < 0)
  {
      Serial.println("Unable to get the azure iot connection string from EEPROM. Please set the value in configuration mode.");
      return kGeneralErr;
  }

  char *iothub_hostname = GetHostNameFromConnectionString((char *)connString);
  char *iothub_deviceid = GetDeviceNameFromConnectionString((char *)connString);

  json_object_set_string(root_object, "iothub", iothub_hostname);
  json_object_set_string(root_object, "iotdevicename", iothub_deviceid);
  json_object_set_string(root_object, "iotdevicesecret", (char *)connString);
  serialized_string = json_serialize_to_string_pretty(root_value);
  puts(serialized_string);

  int json_length = strlen(serialized_string);
    
  err = httpd_send_all_header(req, HTTP_RES_200, json_length, HTTP_CONTENT_JSON_STR);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http headers.") );

  err = httpd_send_body(req->sock, (const unsigned char*)serialized_string, json_length);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http body.") );

exit:
  if (serialized_string)
  {
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
  }

  if (iothub_hostname)
  {
    free(iothub_hostname);
  }

  if (iothub_deviceid)
  {
    free(iothub_deviceid);
  }
  return err;
}

static int RestIoTHubPostHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);

  EEPROMInterface eeprom;
  OSStatus err = kNoErr;
  int buf_size = 1000;
  char *buf;

  buf = (char *)malloc(buf_size);
  err = httpd_get_data(req, buf, buf_size);
  app_httpd_log("httpd_get_data return value: %d", err);
  require_noerr( err, Save_Out );
  
  if (strstr(req->content_type, HTTP_CONTENT_JSON_STR) != NULL)
  {
    JSON_Value *root_value = NULL;
    root_value = json_parse_string(buf);
    JSON_Object *root_object;

    if (json_value_get_type(root_value) == JSONObject)
    {
        root_object = json_value_get_object(root_value);
        const char *strConnString = json_object_get_string(root_object, "connectionstring");

        err = write_eeprom((char *)strConnString, AZ_IOT_HUB_ZONE_IDX);

        if (err != 0)
        {
          return false;
        }

        Serial.println(strConnString);

        if(root_value)
        {
          json_value_free(root_value);
        }
    }
  }

Save_Out:

  err = httpd_send_all_header(req, HTTP_RES_200, 0, HTTP_CONTENT_JSON_STR);

exit:
  return err;  
}

/*
* REST API for WiFi
*/

static int RestWiFiGetHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);

  EEPROMInterface eeprom;
  char ssid[WIFI_SSID_MAX_LEN + 1] = { 0 };
  char pwd[WIFI_PWD_MAX_LEN + 1] = { 0 };

  int ret = eeprom.read((uint8_t*)ssid, WIFI_SSID_MAX_LEN, 0x00, WIFI_SSID_ZONE_IDX);

  if (ret < 0)
  {
      Serial.print("ERROR: Failed to get the Wi-Fi SSID from EEPROM.\r\n");
      return false;
  }

  ret = eeprom.read((uint8_t*)pwd, WIFI_PWD_MAX_LEN, 0x00, WIFI_PWD_ZONE_IDX);
  if (ret < 0)
  {
      Serial.print("ERROR: Failed to get the Wi-Fi password from EEPROM.\r\n");
      return false;
  }

  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *serialized_string = NULL;

  json_object_set_string(root_object, "ssid", ssid);
  json_object_set_string(root_object, "password", pwd);
  serialized_string = json_serialize_to_string_pretty(root_value);
  puts(serialized_string);

  int json_length = strlen(serialized_string);
  int err = kNoErr;
    
  err = httpd_send_all_header(req, HTTP_RES_200, json_length, HTTP_CONTENT_JSON_STR);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http headers.") );

  err = httpd_send_body(req->sock, (const unsigned char*)serialized_string, json_length);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http body.") );

exit:
  if (serialized_string)
  {
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
  }
  return err;
}

static int RestWiFiPostHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  OSStatus err = kNoErr;
  int buf_size = 1000;
  char *buf;

  buf = (char *)malloc(buf_size);
  err = httpd_get_data(req, buf, buf_size);
  app_httpd_log("httpd_get_data return value: %d", err);
  require_noerr( err, Save_Out );
  
  if (strstr(req->content_type, HTTP_CONTENT_JSON_STR) != NULL)
  {
    JSON_Value *root_value = NULL;
    root_value = json_parse_string(buf);
    JSON_Object *root_object;

    if (json_value_get_type(root_value) == JSONObject)
    {
      root_object = json_value_get_object(root_value);
      const char *strSSID = json_object_get_string(root_object, "ssid");
      const char *strPASS = json_object_get_string(root_object, "password");
      
      err = write_eeprom((char *)strSSID, WIFI_SSID_ZONE_IDX);
      if (err != 0)
      {
        return false;
      }
      err = write_eeprom((char *)strPASS, WIFI_PWD_ZONE_IDX);
      if (err != 0)
      {
        return false;
      }

      if(root_value)
      {
        json_value_free(root_value);
      }
    }
  }

Save_Out:

  err = httpd_send_all_header(req, HTTP_RES_200, 0, HTTP_CONTENT_JSON_STR);

exit:
  return err;  
}

/*
* REST API for timeserver
*/
static int RestTimeServerGetHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *serialized_string = NULL;
  //
  // ToDo: Read Time Server Setting from EEPROM
  //
  json_object_set_string(root_object, "timeserver", "MyTimeServer");
  serialized_string = json_serialize_to_string_pretty(root_value);
  puts(serialized_string);

  int json_length = strlen(serialized_string);
  int err = kNoErr;
    
  err = httpd_send_all_header(req, HTTP_RES_200, json_length, HTTP_CONTENT_JSON_STR);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http headers.") );

  err = httpd_send_body(req->sock, (const unsigned char*)serialized_string, json_length);
  require_noerr_action( err, exit, app_httpd_log("ERROR: Unable to send http body.") );

exit:
  if (serialized_string)
  {
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
  }
  return err;
}

static int RestTimeServerPostHandler(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  OSStatus err = kNoErr;
  int buf_size = 1000;
  char *buf;

  buf = (char *)malloc(buf_size);
  err = httpd_get_data(req, buf, buf_size);
  app_httpd_log("httpd_get_data return value: %d", err);
  require_noerr( err, Save_Out );
  
  if (strstr(req->content_type, HTTP_CONTENT_JSON_STR) != NULL)
  {
    //
    // ToDo : Save time server to EEPROM
    //
    JSON_Value *root_value = NULL;
    root_value = json_parse_string(buf);
    JSON_Object *root_object;

    if (json_value_get_type(root_value) == JSONObject)
    {
      root_object = json_value_get_object(root_value);
      const char *strTimeServer = json_object_get_string(root_object, "timeserver");

      Serial.println(strTimeServer);

      if(root_value)
      {
        json_value_free(root_value);
      }
    }
  }

Save_Out:

  err = httpd_send_all_header(req, HTTP_RES_200, 0, HTTP_CONTENT_JSON_STR);

exit:
  return err;  
}

static int RestShutdownPost(httpd_request_t *req)
{
  Serial.println(__FUNCTION__);
  OSStatus err = kNoErr;
  int buf_size = 1000;
  char *buf;

  buf = (char *)malloc(buf_size);
  err = httpd_get_data(req, buf, buf_size);
  app_httpd_log("httpd_get_data return value: %d", err);
  require_noerr( err, Save_Out );
  
  if (strstr(req->content_type, HTTP_CONTENT_JSON_STR) != NULL)
  {
    JSON_Value *root_value = NULL;
    root_value = json_parse_string(buf);
    JSON_Object *root_object;

    if (json_value_get_type(root_value) == JSONObject)
    {
      root_object = json_value_get_object(root_value);
      double iDelay = json_object_get_number(root_object, "shutdowndelayinms");

      if(root_value)
      {
        json_value_free(root_value);
      }

      delay(iDelay);
      mico_system_reboot();
    }
  }

Save_Out:

  err = httpd_send_all_header(req, HTTP_RES_200, 0, HTTP_CONTENT_JSON_STR);

exit:
  return err;  
}

static struct httpd_wsgi_call g_app_handlers[] =
{
	{ "/bootstrap-grid.min.css", HTTPD_HDR_DEFORT, 0, CssBootstrapGrid            , NULL                          , NULL, NULL },
	{ "/rebutton.css"          , HTTPD_HDR_DEFORT, 0, CssRebutton                 , NULL                          , NULL, NULL },
	{ "/"                      , HTTPD_HDR_DEFORT, 0, HtmlIndexGetHandler         , NULL                          , NULL, NULL },
	{ "/wifi"                  , HTTPD_HDR_DEFORT, 0, HtmlWiFiGetHandler          , NULL                          , NULL, NULL },
	{ "/wifi2"                 , HTTPD_HDR_DEFORT, 0, NULL                        , HtmlWiFi2PostHandler          , NULL, NULL },
	{ "/iotcentral"            , HTTPD_HDR_DEFORT, 0, HtmlIoTCentralGetHandler    , NULL                          , NULL, NULL },
	{ "/iotcentral2"           , HTTPD_HDR_DEFORT, 0, NULL                        , HtmlIoTCentral2PostHandler    , NULL, NULL },
	{ "/iothub"                , HTTPD_HDR_DEFORT, 0, HtmlIoTHubGetHandler        , NULL                          , NULL, NULL },
	{ "/iothub2"               , HTTPD_HDR_DEFORT, 0, NULL                        , HtmlIoTHub2PostHandler        , NULL, NULL },
	{ "/message"               , HTTPD_HDR_DEFORT, 0, HtmlMessageGetHandler       , NULL                          , NULL, NULL },
	{ "/message2"              , HTTPD_HDR_DEFORT, 0, NULL                        , HtmlMessage2PostHandler       , NULL, NULL },
	{ "/apmode"                , HTTPD_HDR_DEFORT, 0, HtmlAPmodeGetHandler        , NULL                          , NULL, NULL },
	{ "/apmode2"               , HTTPD_HDR_DEFORT, 0, NULL                        , HtmlAPmode2PostHandler        , NULL, NULL },
	{ "/firmware"              , HTTPD_HDR_DEFORT, 0, HtmlFirmwareUpdateGetHandler, NULL                          , NULL, NULL },
	{ "/firmware2"             , HTTPD_HDR_DEFORT, 0, NULL                        , HtmlFirmwareUpdate2PostHandler, NULL, NULL },
	{ "/shutdown"              , HTTPD_HDR_DEFORT, 0, HtmlShutdownGetHandler      , NULL                          , NULL, NULL },
	//{ "/api"           , HTTPD_HDR_DEFORT, 0, RestConfigGetHandler        , RestConfigPostHandler         , NULL, NULL },
	//{ "/api/iothub"    , HTTPD_HDR_DEFORT, 0, RestIoTHubGetHandler        , RestIoTHubPostHandler         , NULL, NULL },
	//{ "/api/wifi"      , HTTPD_HDR_DEFORT, 0, RestWiFiGetHandler          , RestWiFiPostHandler           , NULL, NULL },
	//{ "/api/timeserver", HTTPD_HDR_DEFORT, 0, RestTimeServerGetHandler    , RestTimeServerPostHandler     , NULL, NULL },
	//{ "/api/shutdown"  , HTTPD_HDR_DEFORT, 0, NULL                        , RestShutdownPost              , NULL, NULL },
};

static int g_app_handlers_no = sizeof(g_app_handlers) / sizeof(g_app_handlers[0]);

static void app_http_register_handlers()
{
  int rc;
  rc = httpd_register_wsgi_handlers(g_app_handlers, g_app_handlers_no);
  if (rc)
  {
    app_httpd_log("failed to register test web handler");
  }
}

static int _app_httpd_start()
{
  OSStatus err = kNoErr;
  app_httpd_log("initializing web-services");

  /*Initialize HTTPD*/
  if (is_http_init == false)
  {
    err = httpd_init();
    require_noerr_action( err, exit, app_httpd_log("failed to initialize httpd") );
    is_http_init = true;
  }

  /*Start http thread*/
  err = httpd_start();
  if (err != kNoErr)
  {
    app_httpd_log("failed to start httpd thread");
    httpd_shutdown();
  }
exit:
  return err;
}

int HttpServerStart()
{
  int err = kNoErr;
  err = _app_httpd_start();
  require_noerr( err, exit );

  if (is_handlers_registered == false)
  {
    app_http_register_handlers();
    is_handlers_registered = true;
  }

exit:
  return err;
}

int HttpServerStop()
{
  OSStatus err = kNoErr;

  /* HTTPD and services */
  app_httpd_log("stopping down httpd");
  err = httpd_stop();
  require_noerr_action( err, exit, app_httpd_log("failed to halt httpd") );

exit:
  return err;
}
