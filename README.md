# xshop

UserManager
 - register
 - login
 - addFund
 - changePassword
 - getCurrentUser
 - currentUserCan
 |
User
 - username
 - password
 - balance
 - setDiscount(merchant)
 > UserConsumer
    - type = consumer
 > UserMerchant
    - type = merchant

ProductManager
 - listProduct
 - addProduct(merchant)
 - editProduct(merchant)
 - deleteProduct(merchant)
 | 
Product
 - name
 - description
 - price
 - stock
 - owner
 > ProductBook
    - type = book
 > ProductFood
    - type = food
 > ProductCloth
    - type = cloth

v1
OrderManager
 - createOrder

v2
CartManager
 - listCart
 - setProductQuantity

OrderManager
 - listOrders
 - createOrder
 - payOrder