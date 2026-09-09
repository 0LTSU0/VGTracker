import 'package:flutter/material.dart';

import '../../models/profile_response.dart';
import '../../services/profile_service.dart';

class ProfileScreen extends StatefulWidget {
  final int userId;

  const ProfileScreen({super.key, required this.userId});

  @override
  State<ProfileScreen> createState() => _ProfileScreenState();
}

class _ProfileScreenState extends State<ProfileScreen> {
  final ProfileService _profileService = ProfileService();

  ProfileResponse? _profile;
  Object? _error;
  bool _isLoading = true;

  @override
  void initState() {
    super.initState();
    _loadProfile();
  }

  Future<void> _loadProfile() async {
    setState(() {
      _isLoading = true;
      _error = null;
    });

    try {
      final profile = await _profileService.getProfile();

      if (!mounted) return;

      setState(() {
        _profile = profile;
      });
    } catch (error) {
      if (!mounted) return;

      setState(() {
        _error = error;
      });
    } finally {
      if (mounted) {
        setState(() {
          _isLoading = false;
        });
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    return _buildBody();
  }

  Widget _buildBody() {
    if (_isLoading) {
      return const Center(child: CircularProgressIndicator());
    }

    if (_error != null) {
      return Center(
        child: Padding(
          padding: const EdgeInsets.all(24),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              Text(
                _error.toString().replaceFirst('Exception: ', ''),
                textAlign: TextAlign.center,
              ),
              const SizedBox(height: 16),
              ElevatedButton(
                onPressed: _loadProfile,
                child: const Text('Try again'),
              ),
            ],
          ),
        ),
      );
    }

    final profile = _profile!;
    return RefreshIndicator(
      onRefresh: _loadProfile,
      child: Center(
        child: ListView(
          shrinkWrap: true,
          padding: const EdgeInsets.all(24),
          children: [
            Text(
              profile.username,
              textAlign: TextAlign.center,
              style: const TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 12),
            Text('Email: ${profile.email}', textAlign: TextAlign.center),
            const SizedBox(height: 8),
            Text('User ID: ${profile.id}', textAlign: TextAlign.center),
          ],
        ),
      ),
    );
  }
}
