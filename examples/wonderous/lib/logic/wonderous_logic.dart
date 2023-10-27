import 'package:wonderous/common_libs.dart';
import 'package:wonderous/logic/data/wonder_data.dart';
import 'package:wonderous/logic/data/wonderous_data/chichen_itza_data.dart';
import 'package:wonderous/logic/data/wonderous_data/christ_redeemer_data.dart';
import 'package:wonderous/logic/data/wonderous_data/colosseum_data.dart';
import 'package:wonderous/logic/data/wonderous_data/great_wall_data.dart';
import 'package:wonderous/logic/data/wonderous_data/machu_picchu_data.dart';
import 'package:wonderous/logic/data/wonderous_data/petra_data.dart';
import 'package:wonderous/logic/data/wonderous_data/pyramids_giza_data.dart';
import 'package:wonderous/logic/data/wonderous_data/taj_mahal_data.dart';

class WonderousLogic {
  List<WonderData> all = [];

  final int timelineStartYear = -3000;
  final int timelineEndYear = 2200;

  WonderData getData(WonderType value) {
    WonderData? result = all.firstWhereOrNull((w) => w.type == value);
    if (result == null) throw ('Could not find data for wonder type $value');
    return result;
  }

  void init() {
    all = [
      GreatWallData(),
      PetraData(),
      ColosseumData(),
      ChichenItzaData(),
      MachuPicchuData(),
      TajMahalData(),
      ChristRedeemerData(),
      PyramidsGizaData(),
    ];
  }
}
