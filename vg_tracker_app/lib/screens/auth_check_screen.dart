import 'package:flutter/material.dart';

import '../services/auth_service.dart';
import 'home/home_screen.dart';
import 'login/login_screen.dart';

class AuthCheckScreen extends StatefulWidget {
  const AuthCheckScreen({
    super.key,
  });

  @override
  State<AuthCheckScreen> createState() =>
      _AuthCheckScreenState();
}

class _AuthCheckScreenState
    extends State<AuthCheckScreen> {
  final AuthService _authService = AuthService();

  @override
  void initState() {
    super.initState();

    _checkAuthentication();
  }

  Future<void> _checkAuthentication() async {
    final token = await _authService.getToken();
    final userId = await _authService.getUserId();

    if (!mounted) {
      return;
    }

    if (token != null && userId != null) {
      Navigator.of(context).pushReplacement(
        MaterialPageRoute(
          builder: (_) => HomeScreen(
            userId: userId,
          ),
        ),
      );
    } else {
      Navigator.of(context).pushReplacement(
        MaterialPageRoute(
          builder: (_) => const LoginScreen(),
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return const Scaffold(
      body: Center(
        child: CircularProgressIndicator(),
      ),
    );
  }
}