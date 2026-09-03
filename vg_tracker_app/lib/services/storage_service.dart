import 'package:flutter_secure_storage/flutter_secure_storage.dart';

class StorageService {
  static const String _tokenKey = 'auth_token';
  static const String _userIdKey = 'user_id';

  final FlutterSecureStorage _storage =
      const FlutterSecureStorage();

  Future<void> saveAuth({
    required String token,
    required int userId,
  }) async {
    await _storage.write(
      key: _tokenKey,
      value: token,
    );

    await _storage.write(
      key: _userIdKey,
      value: userId.toString(),
    );
  }

  Future<String?> getToken() async {
    return _storage.read(key: _tokenKey);
  }

  Future<int?> getUserId() async {
    final value = await _storage.read(key: _userIdKey);

    if (value == null) {
      return null;
    }

    return int.tryParse(value);
  }

  Future<void> clearAuth() async {
    await _storage.delete(key: _tokenKey);
    await _storage.delete(key: _userIdKey);
  }
}