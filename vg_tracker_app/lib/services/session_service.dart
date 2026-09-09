import 'package:flutter/foundation.dart';

import 'storage_service.dart';

/// Holds the user ID for the active session, or null when signed out.
class SessionService {
  SessionService._();

  static final StorageService _storageService = StorageService();

  static final ValueNotifier<int?> userId = ValueNotifier<int?>(null);

  static Future<void> restoreSession() async {
    final token = await _storageService.getToken();
    final storedUserId = await _storageService.getUserId();

    if (token != null && token.isNotEmpty && storedUserId != null) {
      userId.value = storedUserId;
      return;
    }

    await _storageService.clearAuth();
    userId.value = null;
  }

  static Future<void> startSession({
    required String token,
    required int userId,
  }) async {
    await _storageService.saveAuth(token: token, userId: userId);
    SessionService.userId.value = userId;
  }

  static Future<void> endSession() async {
    await _storageService.clearAuth();
    userId.value = null;
  }
}
