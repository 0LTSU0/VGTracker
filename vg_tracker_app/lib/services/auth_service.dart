import 'dart:convert';

import '../config/api_config.dart';
import '../models/login_response.dart';
import 'api_service.dart';
import 'storage_service.dart';

class AuthService {
  final ApiService _apiService = ApiService();
  final StorageService _storageService = StorageService();

  Future<LoginResponse> login({
    required String email,
    required String password,
  }) async {
    final response = await _apiService.post(
      '${ApiConfig.loginEndpoint}?token_in_json=true',
      {
        'email': email,
        'password': password,
      },
      authenticated: false,
    );

    final data = jsonDecode(response.body);

    if (response.statusCode == 200) {
      final result = LoginResponse.fromJson(data);

      await _storageService.saveAuth(
        token: result.token,
        userId: result.userId,
      );

      return result;
    }

    if (response.statusCode == 401) {
      throw Exception(
        'Invalid email/username or password',
      );
    }

    throw Exception(
      data is Map && data['detail'] != null
          ? data['detail'].toString()
          : 'Login failed',
    );
  }

  Future<String?> getToken() async {
    return _storageService.getToken();
  }

  Future<int?> getUserId() async {
    return _storageService.getUserId();
  }

  Future<void> logout() async {
    await _storageService.clearAuth();
  }
}