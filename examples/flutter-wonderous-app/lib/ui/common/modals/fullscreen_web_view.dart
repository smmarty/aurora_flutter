import 'package:wonders/common_libs.dart';

class FullscreenWebView extends StatelessWidget {
  const FullscreenWebView(this.url, {super.key});
  final String url;

  @override
  Widget build(BuildContext context) {
    return SafeArea(
      child: Scaffold(
        appBar: AppBar(),
        body: Placeholder(),
      ),
    );
  }
}
