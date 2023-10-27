import 'package:wonderous/common_libs.dart';
import 'package:wonderous/logic/data/timeline_data.dart';

class TimelineLogic {
  List<TimelineEvent> events = [];

  void init() {
    // Create an event for each wonder, and merge it with the list of GlobalEvents
    events = [
      ...GlobalEventsData().globalEvents,
      ...wonderousLogic.all.map(
        (w) => TimelineEvent(
          w.startYr,
          $strings.timelineLabelConstruction(w.title),
        ),
      )
    ];
  }
}
