import 'package:flutter/material.dart';

import '../services/session_service.dart';
import 'home/home_screen.dart';
import 'login/login_screen.dart';

class AuthCheckScreen extends StatefulWidget {
  const AuthCheckScreen({super.key});

  @override
  State<AuthCheckScreen> createState() => _AuthCheckScreenState();
}

class _AuthCheckScreenState extends State<AuthCheckScreen> {
  bool _isCheckingSession = true;

  @override
  void initState() {
    super.initState();

    _restoreSession();
  }

  Future<void> _restoreSession() async {
    await SessionService.restoreSession();

    if (!mounted) {
      return;
    }

    setState(() => _isCheckingSession = false);
  }

  @override
  Widget build(BuildContext context) {
    if (_isCheckingSession) {
      return const Scaffold(body: Center(child: CircularProgressIndicator()));
    }

    return ValueListenableBuilder<int?>(
      valueListenable: SessionService.userId,
      builder: (context, userId, _) {
        if (userId == null) {
          return const LoginScreen();
        }

        return HomeScreen(userId: userId);
      },
    );
  }
}
