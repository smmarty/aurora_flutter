import 'package:wonders/common_libs.dart';
import 'package:wonders/logic/data/wonder_data.dart';
import 'package:wonders/ui/common/controls/app_header.dart';

class FullscreenMapsViewer extends StatelessWidget {
  const FullscreenMapsViewer({super.key, required this.type});

  final WonderType type;

  WonderData get data => wondersLogic.getData(type);

  @override
  Widget build(BuildContext context) {
    return Stack(
      children: const [
        SafeArea(
          top: false,
          child: Center(
            child: Text('Maps not supported.'),
          ),
        ),
        AppHeader(isTransparent: true),
      ],
    );
  }
}
