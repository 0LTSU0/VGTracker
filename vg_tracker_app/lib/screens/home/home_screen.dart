import 'package:flutter/material.dart';

import '../collection/collection_screen.dart';
import '../profile/profile_screen.dart';

class HomeScreen extends StatefulWidget {
  final int userId;

  const HomeScreen({super.key, required this.userId});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  int _selectedIndex = 0;

  @override
  Widget build(BuildContext context) {
    const titles = ['Collection', 'Profile'];

    return Scaffold(
      appBar: AppBar(title: Text(titles[_selectedIndex])),
      body: IndexedStack(
        index: _selectedIndex,
        children: [
          const CollectionScreen(),
          ProfileScreen(userId: widget.userId),
        ],
      ),
      bottomNavigationBar: NavigationBar(
        height: 56,
        selectedIndex: _selectedIndex,
        onDestinationSelected: (index) {
          setState(() {
            _selectedIndex = index;
          });
        },
        destinations: const [
          NavigationDestination(
            icon: Icon(Icons.videogame_asset_outlined),
            selectedIcon: Icon(Icons.videogame_asset),
            label: 'Collection',
          ),
          NavigationDestination(
            icon: Icon(Icons.person_outline),
            selectedIcon: Icon(Icons.person),
            label: 'Profile',
          ),
        ],
      ),
    );
  }
}
