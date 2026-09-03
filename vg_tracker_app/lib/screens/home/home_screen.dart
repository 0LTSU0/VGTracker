import 'package:flutter/material.dart';

class HomeScreen extends StatelessWidget {
  final int userId;

  const HomeScreen({
    super.key,
    required this.userId,
  });

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Home'),
      ),
      body: Center(
        child: Text(
          'Logged in as user $userId',
          style: const TextStyle(
            fontSize: 20,
          ),
        ),
      ),
    );
  }
}