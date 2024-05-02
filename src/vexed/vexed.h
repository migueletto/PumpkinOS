/*  Vexed - Vexed.h - "Program defines"
    Copyright (C) 1999 James McCombe (cybertube@earthling.net)
	 Copyright (C) 2006 The Vexed Source Forge Team

    This file is part of Vexed.

    Vexed is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Vexed is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vexed; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


// NOTE: You need to bump the vexed.rcp version string too!
#define VEXED_VERSION			2
#define VEXED_REVISION			2
#define VEXED_RELEASE			0

// NOTE: Keep the old names here (commented out of course)
//       for reference while releasing new version
#define VEXED_VERSION_NAME		"When ROM Tag matters"			// 2.2

//..................................................... Debugging
//#define ERROR_CHECK_LEVEL ERROR_CHECK_FULL

//..................................................... Program
#define IconID                         1001
#define VersionID                      1002
#define ApplicationID                  1003

//..................................................... String
#define StringInfo                     1100
#define StringPrefsInfo                1101
#define StringSelectLevelInfo          1102
#define StringCredits                  1103

//..................................................... Alert
#define AlertResetYesNo                1200
#define AlertNoReplayCurrent           1201
#define AlertNoReplayPrevious          1202
#define AlertNoPreviousLevel           1203
#define AlertLoadFailed                1204
#define AlertNoLevelPacksInstalled		1205
#define AlertDeleteLevelPack				1206
#define AlertCannotDeleteCurrent			1207
#define AlertNoSolution						1208
#define AlertShowSolution					1209
#define AlertOldPrefsFound             1210
#define AlertLevelNotSolved            1211
#define AlertNoBeam                    1212
#define AlertNoHiRes                   1213
#define AlertConfirmLevelRestart			1214
#define AlertConfirmLevelChange			1215
#define AlertNoBrightnessAdjust			1216
#define AlertNoWebBrowserFound			1217
#define AlertConfirmWebBrowsing			1218

#define AlertDebug							1299

//..................................................... Menu
#define Menu1038                       1300
#define ItemPreferences                1301
#define ItemHowToPlay                  1302
#define ItemAbout                      1303
#define ItemClear                      1304
#define ItemAdjust                     1305
#define ItemSkip                       1306
#define ItemInfo                       1307
#define ItemReplayCurrent              1308
#define ItemReplayPrevious             1309
#define ItemLevelPacks                 1310
#define ItemSolution                   1311
#define ItemBeamVexed						1312
#define ItemCheckForUpdate					1313
#define ItemAdjustBrightness				1314
#define ItemAdjustContrast					1315

//..................................................... Bitmap
// Be careful rearranging these, there are code dependencies on the order
// This first set of 19 are 4 and 8 bit
#define firstBitmap                    1401
#define BitmapWall                     1401
#define BitmapVexed                    1402
#define BitmapIntroOffset              2
#define BitmapIntroV                   1403
#define BitmapIntroE                   1404
#define BitmapIntroX                   1405
#define BitmapIntroE2                  1406
#define BitmapIntroD                   1407
#define BitmapIntroT                   1408
#define BitmapIntroO                   1409
#define BitmapIntroW                   1410
#define BitmapFirst                    1411
#define BitmapBack                     1412
#define BitmapSelectLevel              1413
#define BitmapForward                  1414
#define BitmapLast                     1415
#define BitmapUndo	                  1416
#define BitmapRecall                   1417
#define BitmapMemory                   1418
#define BitmapRestart                  1419
#define BitmapPending                  1420
#define BitmapCount                    20
// This set of 19 is 1 bit mono
#define MonoBMOffset                   20
#define MBitmapWall                    1421
#define MBitmapVexed                   1422
#define MBitmapIntroV                  1423
#define MBitmapIntroE                  1424
#define MBitmapIntroX                  1425
#define MBitmapIntroE2                 1426
#define MBitmapIntroD                  1427
#define MBitmapIntroT                  1428
#define MBitmapIntroO                  1429
#define MBitmapIntroW                  1430
#define MBitmapFirst                   1431
#define MBitmapBack                    1432
#define MBitmapSelectLevel             1433
#define MBitmapForward                 1434
#define MBitmapLast                    1435
#define MBitmapUndo	                  1436
#define MBitmapRecall                  1437
#define MBitmapMemory                  1438
#define MBitmapRestart                 1439
#define MBitmapPending                 1440
// This apparent overlap of 1431 is OK, used to offset to 1441 and up
#define BitmapBS_Extras                1431
#define MBitmapBS_Extras               1491
// Game blocks start here
#define BitmapBS1_1                    1441
#define BitmapBS1_2                    1442
#define BitmapBS1_3                    1443
#define BitmapBS1_4                    1444
#define BitmapBS1_5                    1445
#define BitmapBS1_6                    1446
#define BitmapBS1_7                    1447
#define BitmapBS1_8                    1448
#define BitmapBS2_1                    1451
#define BitmapBS2_2                    1452
#define BitmapBS2_3                    1453
#define BitmapBS2_4                    1454
#define BitmapBS2_5                    1455
#define BitmapBS2_6                    1456
#define BitmapBS2_7                    1457
#define BitmapBS2_8                    1458
#define BitmapBS3_1                    1461
#define BitmapBS3_2                    1462
#define BitmapBS3_3                    1463
#define BitmapBS3_4                    1464
#define BitmapBS3_5                    1465
#define BitmapBS3_6                    1466
#define BitmapBS3_7                    1467
#define BitmapBS3_8                    1468
#define BitmapBS4_1                    1471
#define BitmapBS4_2                    1472
#define BitmapBS4_3                    1473
#define BitmapBS4_4                    1474
#define BitmapBS4_5                    1475
#define BitmapBS4_6                    1476
#define BitmapBS4_7                    1477
#define BitmapBS4_8                    1478
#define BitmapBS5_1                    1481
#define BitmapBS5_2                    1482
#define BitmapBS5_3                    1483
#define BitmapBS5_4                    1484
#define BitmapBS5_5                    1485
#define BitmapBS5_6                    1486
#define BitmapBS5_7                    1487
#define BitmapBS5_8                    1488
#define BitmapBS6_1                    1491
#define BitmapBS6_2                    1492
#define BitmapBS6_3                    1493
#define BitmapBS6_4                    1494
#define BitmapBS6_5                    1495
#define BitmapBS6_6                    1496
#define BitmapBS6_7                    1497
#define BitmapBS6_8                    1498
#define MBitmapBS1_1                   1501
#define MBitmapBS1_2                   1502
#define MBitmapBS1_3                   1503
#define MBitmapBS1_4                   1504
#define MBitmapBS1_5                   1505
#define MBitmapBS1_6                   1506
#define MBitmapBS1_7                   1507
#define MBitmapBS1_8                   1508
#define MBitmapBS2_1                   1511
#define MBitmapBS2_2                   1512
#define MBitmapBS2_3                   1513
#define MBitmapBS2_4                   1514
#define MBitmapBS2_5                   1515
#define MBitmapBS2_6                   1516
#define MBitmapBS2_7                   1517
#define MBitmapBS2_8                   1518
#define MBitmapBS3_1                   1521
#define MBitmapBS3_2                   1522
#define MBitmapBS3_3                   1523
#define MBitmapBS3_4                   1524
#define MBitmapBS3_5                   1525
#define MBitmapBS3_6                   1526
#define MBitmapBS3_7                   1527
#define MBitmapBS3_8                   1528
#define kidrBitmapCongrats             1530
#define BitmapThumbUp						1531
#define BitmapThumbDown						1532

//..................................................... Form
#define FormMain                       1550
#define FormIntro                      1551
#define FormAbout                      1552
#define FormPrefs                      1553
#define FormSelectLevel                1554
#define FormGetLevel                   1555
#define FormInfo                       1556
#define FormIntroNew                   1557
#define FormLevelPack                  1558
#define FormSolution                   1559

//..................................................... Controls
#define ButtonOK                       1600
#define CheckBoxPieceAnim              1601
#define CheckBoxGravityAnim            1602
#define CheckBoxEliminationAnim        1603
#define CheckBoxBlockCheck             1604
#define CheckBoxColorIcons             1605
#define ButtonOK2                      1606
#define ButtonCancel                   1607
#define CheckBoxSound                  1608
#define DisplayGotoLevel               1609
#define DisplaySolved                  1610
#define ButtonSelectLevelOK            1611
#define ButtonSelectLevelCancel        1612
#define ButtonRepeatUp	               1613
#define ButtonRepeatDown               1614
#define InfoButtonOK	                  1615
#define CheckBoxBlindsEffect           1616
#define CheckBoxSkipIntro              1617
#define ButtonOK3                      1618
#define ButtonCredits                  1619
#define ButtonDelete                   1620
#define ListLevelPacks                 1621
#define FieldDescription               1622
#define FieldName                      1623
#define FieldURL                       1624
#define ButtonBeam							1625
#define PopupBlockSet                  1626
#define PopupBlockSetList              1627
#define ButtonLeft                     1628
#define ButtonRight                    1629
#define ButtonDone                     1630
#define FieldLevelPackName             1631
#define ListSelectLevel                1632
#define CheckBoxSaveSolutions          1633
//#define CheckConfirmRestart            1635

//..................................................... Labels
#define Label1008                      1700
#define Label1009                      1701
#define Label1011                      1702
#define Label1073                      1703
#define Label1074                      1704
#define Label1061                      1705
#define Label1098                      1706
#define Label1041                      1707
#define Label1030                      1708
#define LabelSolvedLevels              1709
#define LabelGetSolvedLevels           1710
#define LabelBlock1Count	            1711
#define LabelBlock2Count	            1712
#define LabelBlock3Count	            1713
#define LabelBlock4Count	            1714
#define LabelBlock5Count	            1715
#define LabelBlock6Count	            1716
#define LabelBlock7Count	            1717
#define LabelBlock8Count	            1718

//..................................................... Misc
#define MakeMoveEvent                  0x6000

