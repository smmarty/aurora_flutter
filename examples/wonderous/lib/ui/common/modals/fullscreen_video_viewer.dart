import 'package:pointer_interceptor/pointer_interceptor.dart';
import 'package:wonderous/common_libs.dart';

class FullscreenVideoViewer extends StatefulWidget {
  const FullscreenVideoViewer({Key? key, required this.id}) : super(key: key);
  final String id;

  @override
  State<FullscreenVideoViewer> createState() => _FullscreenVideoViewerState();
}

class _FullscreenVideoViewerState extends State<FullscreenVideoViewer> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.black,
      body: Stack(
        children: [
          Center(
            child: Placeholder(),
          ),
          SafeArea(
            child: Padding(
              padding: EdgeInsets.all($styles.insets.md),
              // Wrap btn in a PointerInterceptor to prevent the HTML video player from intercepting the pointer (https://pub.dev/packages/pointer_interceptor)
              child: PointerInterceptor(
                child: const BackBtn(),
              ),
            ),
          ),
        ],
      ),
    );
  }
}
