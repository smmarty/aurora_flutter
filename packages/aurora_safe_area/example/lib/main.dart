import 'package:aurora_safe_area/aurora_safe_area.dart';
import 'package:flutter/material.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return AuroraSafeArea(
      child: MaterialApp(
        title: 'Flutter Demo',
        theme: ThemeData(
          colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
          useMaterial3: true,
        ),
        home: const MyHomePage(title: 'Aurora Safe Area Playground'),
      ),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  bool enableSafeArea = true;

  @override
  Widget build(BuildContext ctx) {
    return Scaffold(
      body: Builder(builder: (context) {
        return SafeArea(
          left: enableSafeArea,
          top: enableSafeArea,
          right: enableSafeArea,
          bottom: enableSafeArea,
          child: Column(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: <Widget>[
              Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  // Отступ, чтоб показывать границы SafeArea
                  const Divider(
                    height: 2,
                    thickness: 2,
                  ),
                  Text(
                    widget.title,
                    style: Theme.of(context).textTheme.titleLarge,
                  ),
                  const SizedBox(height: 12),
                  Text(
                    'Global Padding: ${MediaQuery.paddingOf(context)}',
                  ),
                  Text(
                    'Local Padding: ${MediaQuery.paddingOf(ctx)}',
                  ),
                  Text(
                    'ViewInsets: ${MediaQuery.viewInsetsOf(ctx)}',
                  ),
                  Text(
                    'Global ViewPadding: ${MediaQuery.viewPaddingOf(context)}',
                  ),
                  Text(
                    'Local ViewPadding: ${MediaQuery.viewPaddingOf(ctx)}',
                  ),
                  const SizedBox(height: 8),
                  ElevatedButton(
                    onPressed: () {
                      setState(() {
                        enableSafeArea = !enableSafeArea;
                      });
                    },
                    child: Text('SafeArea: $enableSafeArea'),
                  ),
                  const SizedBox(height: 8),
                  ElevatedButton(
                    onPressed: () {
                      showBottomSheet(
                        context: context,
                        builder: (context) => Container(
                          height: 200,
                          color: Colors.amber,
                          child: Center(
                            child: Column(
                              mainAxisAlignment: MainAxisAlignment.center,
                              mainAxisSize: MainAxisSize.min,
                              children: <Widget>[
                                const Text('BottomSheet'),
                                ElevatedButton(
                                  child: const Text('Close BottomSheet'),
                                  onPressed: () {
                                    Navigator.pop(context);
                                  },
                                ),
                              ],
                            ),
                          ),
                        ),
                      );
                    },
                    child: const Text('Открыть модалку'),
                  ),
                  const SizedBox(height: 8),
                ],
              ),
              const Padding(
                padding: EdgeInsets.all(16),
                child: TextField(
                  decoration: InputDecoration(
                    border: OutlineInputBorder(),
                  ),
                ),
              ),
            ],
          ),
        );
      }),
    );
  }
}
